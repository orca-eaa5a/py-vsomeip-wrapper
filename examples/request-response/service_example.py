import vsomeip

# https://github.com/COVESA/vsomeip/wiki/vsomeip-in-10-minutes#request--response


SAMPLE_SERVICE_ID=0x1234
SAMPLE_INSTANCE_ID=0x5678
SAMPLE_METHOD_ID=0x0421


def on_message(val):
    import binascii
    print("SERVICE: Received message with Client/Session {}".format(" ".join("0x{:02x}".format(h) for h in val)))
    return 1

def main():
    vsomeip.create_application("World")
    vsomeip.init()
    vsomeip.register_message_handler(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_METHOD_ID, on_message)
    vsomeip.offer_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID)
    vsomeip.start()

    import time
    time.sleep(15)
    
    pass

if __name__ == '__main__':
    main()
    pass