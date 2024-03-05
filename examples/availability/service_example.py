import vsomeip

# https://github.com/COVESA/vsomeip/wiki/vsomeip-in-10-minutes#availability
# same as service-example.cpp

SAMPLE_SERVICE_ID=0x1234
SAMPLE_INSTANCE_ID=0x5678

def main():
    vsomeip.create_application("World")
    vsomeip.init()
    vsomeip.offer_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID)
    vsomeip.start()

    import time
    time.sleep(10)
    pass

if __name__ == '__main__':
    main()
    pass