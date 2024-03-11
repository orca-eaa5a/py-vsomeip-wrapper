#include <Python.h>

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>
#include <vsomeip/vsomeip.hpp>

// std::shared_ptr<vsomeip::application> app;

/**
 * app_id : std::shared_ptr<vsomeip::application>
 **/
std::map<int, std::shared_ptr<vsomeip::application>> apps;

std::map<int, std::map<int, std::map<int, PyObject *>>> cb_map;

std::map<int, PyObject *> state_cb_map;

/**
 * app_id : {
 *  service_id : std::vector<PyObject *> (callback)
 * }
*/
std::map<int, std::map<int, std::vector<PyObject *>>> availability_handler_vec;

/**
 * service_id : {
 *  instance_id : {
 *    method_id : std::shared_ptr<vsomeip::message> (callback)
 *  }
 * }
 * 
*/
std::map<int, std::map<int, std::map<int, std::shared_ptr<vsomeip::message>>>> request_map;

/**
 * app_id : {
 *  service_id :{
 *    instance_id : std::set<vsomeip::eventgroup_t>
 *  }
 * }
 **/
std::map<int, std::map<int, std::map<int, std::set<vsomeip::eventgroup_t>>>> evtgrp_map;

static void debug_write(const std::string msg) {
  std::cout << msg ;
  std::cout << std::endl;
}

// static void print_request(const std::shared_ptr<vsomeip::message> &_request) {
//   std::shared_ptr<vsomeip::payload> its_payload = _request->get_payload();
//   vsomeip::length_t l = its_payload->get_length();

//   // Get payload
//   std::stringstream ss;
//   for (vsomeip::length_t i = 0; i < l; i++) {
//     ss << std::setw(2) << std::setfill('0') << std::hex
//        << (int)*(its_payload->get_data() + i) << " ";
//   }

//   std::cout << "SERVICE: Received message with Client/Session [" << std::setw(4)
//             << std::setfill('0') << std::hex << _request->get_client() << "/"
//             << std::setw(4) << std::setfill('0') << std::hex
//             << _request->get_session() << "] " << ss.str() << std::endl;
// }

static PyObject *payload_pack(std::shared_ptr<vsomeip::payload> pl) {

  return PyByteArray_FromStringAndSize((const char *)pl->get_data(),
                                       pl->get_length());
}

std::shared_ptr<vsomeip::payload> payload_unpack(PyObject *obj) {

  std::shared_ptr<vsomeip::payload> pl;
  char *raw_data = PyByteArray_AsString(obj);
  Py_ssize_t raw_size = PyByteArray_Size(obj);

  std::vector<vsomeip::byte_t> pl_data(raw_size);
  for (int i = 0; i < raw_size; i++)
    pl_data[i] = raw_data[i];
  pl = vsomeip::runtime::get()->create_payload();
  pl->set_data(pl_data);

  return pl;
}

// PyObject *test_pack_unpack(PyObject *self, PyObject *args) {

//   PyObject *bytes;
//   if (!PyArg_ParseTuple(args, "O", &bytes)) {
//     return NULL;
//   }
//   std::shared_ptr<vsomeip::payload> pl = payload_unpack(bytes);

//   vsomeip::length_t l = pl->get_length();

//   // Get payload
//   std::stringstream ss;
//   for (vsomeip::length_t i = 0; i < l; i++) {
//     ss << std::setw(2) << std::setfill('0') << std::hex
//        << (int)*(pl->get_data() + i) << " ";
//   }

//   std::cout << "PL: " << ss.str() << std::endl;

//   PyObject *obj = payload_pack(pl);

//   return Py_BuildValue("O", obj);
// }

void message_dispatcher(const std::shared_ptr<vsomeip::message> &_msg) {
  // call python callback function
  std::cout << "MESSAGE DISPATCHER Called" << std::endl;
  std::shared_ptr<vsomeip::payload> its_payload = _msg->get_payload();

  // print_request(_msg);
  PyObject *cb = cb_map[_msg->get_service()][_msg->get_instance()][_msg->get_method()]; // get callback function
  if (cb == nullptr){
    cb = cb_map[vsomeip::ANY_SERVICE][vsomeip::ANY_INSTANCE][vsomeip::ANY_METHOD]; // get callback function
    if (cb == nullptr){
      return;
    }
  }
  if (PyCallable_Check(cb)) {
      PyObject *arglist;
      PyObject *bytes;
      bytes = payload_pack(its_payload);
      PyGILState_STATE gstate;
      gstate = PyGILState_Ensure(); // Thread Safe
      arglist = PyTuple_New(1);
      PyTuple_SetItem(arglist, 0, Py_BuildValue("O", bytes));

      std::cout << "Call Python defined callback" << std::endl;
      PyObject *result = PyObject_CallObject(cb, arglist); // call callback object
      if (PyErr_Occurred()) {
          PyErr_PrintEx(0);
          PyErr_Clear(); // this will reset the error indicator so you can run Python code again
      }
      std::cout << "End Python defined callback" << std::endl;
      
      PyGILState_Release(gstate);
  } else {
      PyErr_Print();
      fprintf(stderr, "Not a callable object\n");
  }

  // if (PyObject_TypeCheck(result, &PyByteArray_Type)) {
  //   std::shared_ptr<vsomeip::message> its_response = vsomeip::runtime::get()->create_response(_msg);
  //   std::shared_ptr<vsomeip::payload> payload = payload_unpack(result);
  //   its_response->set_payload(payload);
  //   app->send(its_response, true);
  // }

  std::cout << "MESSAGE DISPATCHER Finished" << std::endl;
}

void availability_handler(vsomeip::service_t _service, vsomeip::instance_t _instance, bool _is_available){
  int svc_id = (int)_service;
  int inst_id = (int)_instance;
  for(PyObject* cb : availability_handler_vec[svc_id][inst_id]){
    if (PyCallable_Check(cb)) {
        PyObject *arglist;
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure(); // Thread Safe
        arglist = PyTuple_New(3);
        PyTuple_SetItem(arglist, 0, Py_BuildValue("i", _service));
        PyTuple_SetItem(arglist, 1, Py_BuildValue("i", _instance));
        PyTuple_SetItem(arglist, 2, Py_BuildValue("i", _is_available)); // "p" is not working

        std::cout << "Call Python defined availability_handler" << std::endl;
        PyObject *result = PyObject_CallObject(cb, arglist); // call callback object
        if (PyErr_Occurred()) {
            std::cout << "Fuckyou" << std::endl;
            PyErr_PrintEx(0);
            PyErr_Clear(); // this will reset the error indicator so you can run Python code again
        }
        std::cout << "Call Python defined availability_handler" << std::endl;
        PyGILState_Release(gstate);
    } else {
        PyErr_Print();
        fprintf(stderr, "Not a callable object\n");
    }
  }
}

static PyObject *vsomeip_create_application(PyObject *self, PyObject *args) {
  int app_id;
  const char *name;
  int sts = 1;

  if (!PyArg_ParseTuple(args, "is", &app_id, &name))
    return NULL;

  apps[app_id] = vsomeip::runtime::get()->create_application(name);

  return Py_BuildValue("i", app_id);
}

static PyObject *vsomeip_init(PyObject *self, PyObject *args){
  int app_id;
  int sts = 1;

  if (!PyArg_ParseTuple(args, "i", &app_id))
    return NULL;
  if(apps.count(app_id) == 0)
    return NULL;
  std::shared_ptr<vsomeip::application> app = apps[app_id];
  
  app->init();
  app.reset(); // free shared ptr;

  return Py_BuildValue("i", sts);
}

static void start(int app_id) { 
  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->start(); 
  app.reset();
}

static PyObject *vsomeip_start(PyObject *self, PyObject *args) {
  int app_id;
  int sts = 1;
  
  if (!PyArg_ParseTuple(args, "i", &app_id))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  std::thread vsomeip_main(&start, app_id);

  vsomeip_main.detach();

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_notify(PyObject *self, PyObject *args) {

  int app_id;
  int svc_id, inst_id, evt;
  PyObject *val;
  int sts = 1;

  if (!PyArg_ParseTuple(args, "iiiiO", &app_id, &svc_id, &inst_id, &evt, &val))
    PyErr_SetString(PyExc_TypeError, "=Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  std::shared_ptr<vsomeip::payload> payload = payload_unpack(val);
  std::shared_ptr<vsomeip::application> app = apps[app_id];

  app->notify(svc_id, inst_id, evt, payload);
  app.reset(); // free shared ptr;

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_offer_event(PyObject *self, PyObject *args) {
  int app_id;
  int svc_id, inst_id, evt_id, evgrp_id;
  vsomeip::event_type_e event_type;
  int sts = 1;

  if (!PyArg_ParseTuple(args, "iiiiii", &app_id, &svc_id, &inst_id, &evt_id, &evgrp_id, &event_type))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  std::set<vsomeip::eventgroup_t> grp;
  evtgrp_map[app_id][svc_id][inst_id] = grp;
  grp.insert(evgrp_id);

  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->offer_event(svc_id, inst_id, evt_id, grp, event_type);
  app.reset(); // free shared ptr;

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_request_event(PyObject *self, PyObject *args) {
  int app_id;
  int svc_id, inst_id, evt_id, evtgrp_id;
  vsomeip::event_type_e event_type;
  vsomeip::reliability_type_e reliability_type;
  int sts = 1;

  // if (!PyArg_ParseTuple(args, "iiii", &svc_id, &inst_id, &evt_id, &grp))
  //   PyErr_SetString(PyExc_TypeError, "Invalid args!");

  if (!PyArg_ParseTuple(args, "iiiiiii", &app_id, &svc_id, &inst_id, &evt_id, &evtgrp_id, &event_type, &reliability_type))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  std::set<vsomeip::eventgroup_t> grp;
  evtgrp_map[app_id][svc_id][inst_id] = grp;
  grp.insert(evtgrp_id);
  
  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->request_event(svc_id, inst_id, evt_id, grp, event_type, reliability_type);
  app.reset();

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_subscribe(PyObject *self, PyObject *args) {
  int app_id;
  int svc_id, inst_id, evtgrp_id;
  int major, event;
  int sts = 1;

  if (!PyArg_ParseTuple(args, "iiii", &app_id, &svc_id, &inst_id, &evtgrp_id, &major, &event))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->subscribe(svc_id, inst_id, evtgrp_id, major, event);
  app.reset();

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_offer_service(PyObject *self, PyObject *args) {
  int app_id;
  int svc_id, inst_id;
  int sts = 0;

  if (!PyArg_ParseTuple(args, "iii", &app_id, &svc_id, &inst_id))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->offer_service(svc_id, inst_id); // major minor version
  app.reset();

  return Py_BuildValue("i", sts);
}

// It is inevitable write callback as C++, not calling python defined function directly, because of the type checking
// registered function on register_availability_handler must following (void)(int, int, vsomeip_v3::availability_state_handler_t)
static PyObject *vsomeip_register_availability_handler(PyObject *self, PyObject *args){
  int app_id;
  int svc_id, inst_id;
  int sts = 1;
  PyObject *pycbobj;

  if (!PyArg_ParseTuple(args, "iiiO", &app_id, &svc_id, &inst_id, &pycbobj))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if (!PyCallable_Check(pycbobj)) {
    PyErr_SetString(PyExc_TypeError, "Need a callable object!");
  }
  if(apps.count(app_id) == 0)
    return NULL;

  Py_XINCREF(pycbobj);
  availability_handler_vec[svc_id][inst_id].push_back(pycbobj);

  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->register_availability_handler(svc_id, inst_id, availability_handler);
  app.reset();
  
  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_register_message_handler(PyObject *self, PyObject *args) {
  int app_id;
  int svc_id, inst_id, mtd_id;
  int sts = 1;
  PyObject *pycbobj;

  if (!PyArg_ParseTuple(args, "iiiiO", &app_id, &svc_id, &inst_id, &mtd_id, &pycbobj))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  // make sure last argument is a function
  if (!PyCallable_Check(pycbobj)) {
    PyErr_SetString(PyExc_TypeError, "Need a callable object!");
  }
  if(apps.count(app_id) == 0)
    return NULL;

  Py_XINCREF(pycbobj);

  cb_map[svc_id][inst_id][mtd_id] = pycbobj;
  
  // std::cout << "Cb function registered" << std::endl;

  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->register_message_handler(svc_id, inst_id, mtd_id, message_dispatcher);
  app.reset();

  return Py_BuildValue("i", sts);
}

/*
static PyObject* vsomeip_register_state_handler(PyObject *self, PyObject *args){
  // it is impossible to implement as C/C++ python extension
  int sts = 1;
  int app_id;
  PyObject* pycbobj;

  if (!PyArg_ParseTuple(args, "iO", &pycbobj))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  Py_XINCREF(pycbobj);
  state_cb_map[app_id] = pycbobj

  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app.reset()

  return Py_BuildValue("i", sts);
}
*/

static PyObject *vsomeip_request_service(PyObject *self, PyObject *args) {
  int app_id;
  int svc_id, inst_id;
  int sts = 1;

  if (!PyArg_ParseTuple(args, "iii", &app_id, &svc_id, &inst_id))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->request_service(svc_id, inst_id);
  app.reset();

  return Py_BuildValue("i", sts);
}

static PyObject *vsomeip_create_request(PyObject *self, PyObject *args){
  int svc_id, inst_id, mtd_id;
  PyObject *val;
  int sts = 1;

  if (!PyArg_ParseTuple(args, "iii", &svc_id, &inst_id, &mtd_id))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");

  std::shared_ptr<vsomeip::message> rq = vsomeip::runtime::get()->create_request();
  
  rq->set_service(svc_id);
  rq->set_instance(inst_id);
  rq->set_method(mtd_id);

  // std::cout << rq.use_count() << std::endl; // debug print

  request_map[svc_id][inst_id][mtd_id] = rq;

  // std::cout << "New Request Created" << std::endl // debug print
  // << typeid(rq).name() << " : " << std::addressof(rq) << std::endl 
  // << "Service ID: " << svc_id << std::endl
  // << "Instance ID: " << inst_id << std::endl
  // << "Method ID: " << mtd_id << std::endl
  // << "------------------------"<< std::endl;

  return Py_BuildValue("L", rq);
}

static PyObject *vsomeip_setup_request_payload(PyObject *self, PyObject *args){
  int svc_id, inst_id, mtd_id;
  PyObject *payload_bytes;
  int sts = 1;

  if (!PyArg_ParseTuple(args, "iiiO", &svc_id, &inst_id, &mtd_id, &payload_bytes))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");

  std::shared_ptr< vsomeip::payload > v_payload_obj = vsomeip::runtime::get()->create_payload();
  char *raw_data = PyByteArray_AsString(payload_bytes);
  Py_ssize_t raw_size = PyByteArray_Size(payload_bytes);

  std::vector<vsomeip::byte_t> pl_data(raw_size);
  for (int i = 0; i < raw_size; i++)
    pl_data[i] = raw_data[i];
  v_payload_obj->set_data(pl_data);

  std::shared_ptr<vsomeip::message> rq = request_map[svc_id][inst_id][mtd_id];

  // std::cout << rq.use_count() << std::endl; // debug print

  rq->set_payload(v_payload_obj);

  // std::cout << "New Request Payload Created" << std::endl // debug print
  // << typeid(rq).name() << " : " << std::addressof(rq) << std::endl 
  // << "Service ID: " << svc_id << std::endl
  // << "Instance ID: " << inst_id << std::endl
  // << "Method ID: " << mtd_id << std::endl
  // << "Payload Size: " << raw_size << std::endl
  // << "------------------------"<< std::endl;

  rq.reset(); // free shared_ptr

  return Py_BuildValue("L", sts);
}

static PyObject *vsomeip_send_request(PyObject *self, PyObject *args) {
  int app_id;
  int sts = 1;
  int svc_id, inst_id, mtd_id;

  if (!PyArg_ParseTuple(args, "iiii", &app_id, &svc_id, &inst_id, &mtd_id))
    PyErr_SetString(PyExc_TypeError, "Invalid args!");
  if(apps.count(app_id) == 0)
    return NULL;

  std::shared_ptr<vsomeip::application> app = apps[app_id];
  app->send(request_map[svc_id][inst_id][mtd_id]);
  app.reset();

  return Py_BuildValue("i", sts);
}

// static PyObject *vsomeip_stop_offer_service(PyObject *self, PyObject *args) {
//   int svc_id, inst_id;
//   int sts = 0;

//   if (!PyArg_ParseTuple(args, "ii", &svc_id, &inst_id))
//     PyErr_SetString(PyExc_TypeError, "Invalid args!");

//   app->stop_offer_service(svc_id, inst_id);

//   return Py_BuildValue("i", sts);
// }

static PyMethodDef VsomeIpMethods[] = {
    {"create_application", vsomeip_create_application, METH_VARARGS, "Create a vsomeip application."},
    {"init", vsomeip_init, METH_VARARGS, "init"},
    {"start", vsomeip_start, METH_VARARGS, "Start vsomeip application."},
    {"offer_service", vsomeip_offer_service, METH_VARARGS, "start offering service."},
    {"register_availability_handler", vsomeip_register_availability_handler, METH_VARARGS, "..."},
    {"register_message_handler", vsomeip_register_message_handler, METH_VARARGS, "..."},
    {"request_service", vsomeip_request_service, METH_VARARGS, "request service."},
    // {"stop_offer_service", vsomeip_stop_offer_service, METH_VARARGS, "start offering service."},
    {"offer_event", vsomeip_offer_event, METH_VARARGS, "start offering event."},
    {"notify", vsomeip_notify, METH_VARARGS, "notify."},
    {"request_event", vsomeip_request_event, METH_VARARGS, "start requesting event."},
    {"subscribe", vsomeip_subscribe, METH_VARARGS, ""},
    // {"test", test_pack_unpack, METH_VARARGS, "notify."},
    {"create_request", vsomeip_create_request, METH_VARARGS, ""},
    {"setup_request_payload", vsomeip_setup_request_payload, METH_VARARGS, ""},
    {"send_request", vsomeip_send_request, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef vsomeipPyDef = {
    PyModuleDef_HEAD_INIT,
    "vsomeip",
    NULL, //doc
    -1, //the module keeps state in global variables
    VsomeIpMethods
};

PyMODINIT_FUNC PyInit_vsomeip(void) {
  auto module = PyModule_Create(&vsomeipPyDef);
  PyEval_InitThreads();
  return module;
}

int main(int argc, char *argv[]) {
  /* Decode and pass argv[0] to the Python interpreter */
  wchar_t *program = Py_DecodeLocale(argv[0], NULL);
  if (program == NULL) {
    std::cerr << "Fatal error: cannot decode argv[0]" << std::endl;
    return -1;
  }
  Py_SetProgramName(program);

  /* Initialize the Python interpreter.  Required. */
  Py_Initialize();

  return 0;
}
