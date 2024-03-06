class EventType(object):
    ET_EVENT = 0x00
    ET_SELECTIVE_EVENT = 0x01
    ET_FIELD = 0x02
    ET_UNKNOWN = 0xFF

class ReliabilityType(object):
    RT_RELIABLE = 0x01
    RT_UNRELIABLE = 0x02
    RT_BOTH = 0x3 # RT_RELIABLE | RT_UNRELIABLE
    RT_UNKNOWN = 0xFF
