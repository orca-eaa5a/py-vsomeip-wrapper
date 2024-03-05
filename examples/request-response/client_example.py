import vsomeip

# https://github.com/COVESA/vsomeip/wiki/vsomeip-in-10-minutes#request--response


SAMPLE_SERVICE_ID=0x1234
SAMPLE_INSTANCE_ID=0x5678
SAMPLE_METHOD_ID=0x0421


def run():
    vsomeip.create_request(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_METHOD_ID)
    vsomeip.setup_request_payload(
        SAMPLE_SERVICE_ID,
        SAMPLE_INSTANCE_ID,
        SAMPLE_METHOD_ID,
        bytearray([0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42])
    )
    vsomeip.send_request(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_METHOD_ID)

def on_message(val):
    print("CLIENT: Received message with Client/Session {}".format(" ".join("{:02x}".format(h) for h in val)))

def main():
    vsomeip.create_application("Hello")
    vsomeip.init()
    vsomeip.request_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID)
    vsomeip.register_message_handler(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_METHOD_ID, on_message)
    
    # request
    vsomeip.start()

    import time
    time.sleep(1)
    run()
    time.sleep(14)
    
    pass

if __name__ == '__main__':
    main()
    pass