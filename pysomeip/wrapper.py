from typing import List
from enums import *
from consts import *
import vsomeip

# TODO
# - clear_all_handler
# - release_service


class VsomeIP(object):
    def __init__(self) -> None:
        self.app_id = 0
        self.apps:List[str] = []
        pass

    def create_application(self, app_name:str) -> int:
        new_app_id = self.app_id
        app_id = vsomeip.create_application(new_app_id, app_name)
        self.app_id += 1
        
        return app_id

    def app_init(self, app_id:int) -> None:
        ret = vsomeip.init(app_id)
        if not ret:
            raise Exception("init app failed")
        pass
    
    def app_start(self, app_id:int):
        ret = vsomeip.start(app_id)
        if not ret:
            raise Exception("start app failed")
        pass

    def notify(self, app_id:int, service_id:int, instance_id:int, event_id:int, payload:bytes):
        ret = vsomeip.notify(app_id, service_id, instance_id, event_id, bytearray(payload))
        if not ret:
            raise Exception("notify failed")
        pass

    def offer_event(self, app_id:int, service_id:int, instance_id:int, event_id:int, event_group_id:int, event_type=EventType.ET_EVENT):
        ret = vsomeip.offer_event(app_id, service_id, instance_id, event_id, event_group_id, event_type)
        if not ret:
            raise Exception("offer_event failed")
        pass

    def request_event(self, app_id:int, service_id:int, instance_id:int, event_id:int, event_group_id:int, event_type=EventType.ET_EVENT, reliability_type=ReliabilityType.RT_UNKNOWN):
        ret = vsomeip.request_event(app_id, service_id, instance_id, event_id, event_group_id, event_type, reliability_type)
        if not ret:
            raise Exception("request_event failed")
        pass

    def subscribe(self, app_id:int, service_id:int, instance_id:int, event_group_id:int, major=DEFAULT_MAJOR, event=ANY_EVENT):
        ret = vsomeip.subscribe(app_id, service_id, instance_id, event_group_id, major, event)
        if not ret:
            raise Exception("subscribe failed")
        pass

    def offer_service(self, app_id:int, service_id:int, instance_id:int):
        ret = vsomeip.offer_service(app_id, service_id, instance_id)
        if not ret:
            raise Exception("request_event failed")
        pass

    def register_availability_handler(self, app_id:int, service_id:int, instance_id:int, on_message:object):
        ret = vsomeip.register_availability_handler(app_id, service_id, instance_id, on_message)
        if not ret:
            raise Exception("register_availability_handler failed")
        pass

    def register_message_handler(self, app_id:int, service_id:int, instance_id:int, method_id:int, on_message:object):
        ret = vsomeip.register_message_handler(app_id, service_id, instance_id, method_id, on_message)
        if not ret:
            raise Exception("register_message_handler failed")
        pass

    def register_state_handler(self, app_id):
        pass

    def request_service(self, app_id:int, service_id:int, instance_id:int):
        ret = vsomeip.request_service(app_id, service_id, instance_id)
        if not ret:
            raise Exception("request_service failed")
        pass

    def create_request(self, service_id:int, instance_id:int, method_id:int):
        """
        create & setup std::shared_ptr<vsomeip::message> object
        """
        ret = vsomeip.create_request(service_id, instance_id, method_id)
        if not ret:
            raise Exception("create_request failed")
        pass

    def setup_request_payload(self, service_id:int, instance_id:int, method_id:int, payload:bytes):
        ret = vsomeip.setup_request_payload(service_id, instance_id, method_id, bytearray(payload))
        if not ret:
            raise Exception("setup_request_payload failed")
        pass

    def send_request(self, app_id:int, service_id:int, instance_id:int, method_id:int):
        ret = vsomeip.send_request(app_id, service_id, instance_id, method_id)
        if not ret:
            raise Exception("send_request failed")
        pass