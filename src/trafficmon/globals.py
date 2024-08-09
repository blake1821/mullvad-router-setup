class TrafficmonGlobals:
    """
    Global constants shared between the C/C++ and Python code.
    Each name must be a valid C identifer, and each value must be str()-ingable.
    These will be written as macro definitions in the global-vars.h header file.
    """
    PRIVILEGED_MARK = 3331