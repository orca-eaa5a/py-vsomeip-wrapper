import vsomeip

# https://github.com/COVESA/vsomeip/wiki/vsomeip-in-10-minutes#availability
# same as client-example.cpp

SAMPLE_SERVICE_ID=0x1234
SAMPLE_INSTANCE_ID=0x5678

def on_availability(service, inst, is_availiable):
    if not is_availiable:
        print("it is not availiable service")
    else:
        print("it is availiable service : Service ID: 0x%x / Instance ID: 0x%x" % (service, inst))

def main():
    vsomeip.create_application("Hello")
    vsomeip.init()
    try:
        vsomeip.register_availability_handler(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, on_availability)
    except Exception as e:
        print(e)
    vsomeip.request_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID)
    vsomeip.start()
    
    import time
    time.sleep(5)

    pass

if __name__ == '__main__':
    main()
    pass