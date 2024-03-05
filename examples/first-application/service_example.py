import vsomeip

# https://github.com/COVESA/vsomeip/wiki/vsomeip-in-10-minutes#first-application

def main():
    vsomeip.create_application("hello")
    vsomeip.init()
    vsomeip.start()
    pass

if __name__ == '__main__':
    main()
    pass