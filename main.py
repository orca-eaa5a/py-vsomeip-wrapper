from pysomeip import wrapper

def main():
    someip = wrapper.VsomeIP()
    new_app_id = someip.create_application(1)
    someip.app_init(new_app_id)
    
    pass

if __name__ == '__main__':
    main()
    pass