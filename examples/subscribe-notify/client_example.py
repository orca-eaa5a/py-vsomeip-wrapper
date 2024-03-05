# https://github.com/COVESA/vsomeip/wiki/vsomeip-in-10-minutes#subscribe--notify
import vsomeip

SAMPLE_SERVICE_ID=0x1234
SAMPLE_INSTANCE_ID=0x5678

SAMPLE_EVENTGROUP_ID=0x4465
SAMPLE_EVENT_ID=0x8778

VSOMEIP_ANY_SERVICE = 0xFFFF
VSOMEIP_ANY_INSTANCE = 0xFFFF
VSOMEIP_ANY_EVENTGROUP = 0xFFFF
VSOMEIP_ANY_METHOD = 0xFFFF

def on_message(val):
    print("CLIENT: Received message with Client/Session {}".format(" ".join("{:02x}".format(h) for h in val)))

def run():
    vsomeip.request_event(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENT_ID, SAMPLE_EVENTGROUP_ID)

def main():
    vsomeip.create_application("Hello")
    vsomeip.init()
    
    vsomeip.request_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID)
    vsomeip.register_message_handler(VSOMEIP_ANY_SERVICE, VSOMEIP_ANY_INSTANCE, VSOMEIP_ANY_METHOD, on_message)
    run()
    vsomeip.start()

    import time
    time.sleep(15)

    pass

if __name__ == '__main__':
    main()
    pass