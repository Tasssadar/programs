#define TURN_VALUE 40

enum moveFlags
{
    MOVE_NONE         = 0x00,
    MOVE_FORWARD      = 0x01,
    MOVE_BACKWARD     = 0x02,
    MOVE_LEFT         = 0x04,
    MOVE_RIGHT        = 0x08,
};

enum rangeFinderPos
{
    FINDER_MIDDLE     = 0xE0,
    FINDER_LEFT       = 0xE2,
    FINDER_RIGHT      = 0xE4
};

#define RANGE_CENTIMETRES 0x51
#define MEM_SIZE 102 // 512/5
#define REC_SIZE 5 // sizeof(Record)

#define CM      35 // on encoders
#define MM      5 // on encoders

enum recordStopEvents
{
    EVENT_TIME               = 1,
    EVENT_SENSOR_LEVEL_HIGHER= 2,
    EVENT_SENSOR_LEVEL_LOWER = 3,
    EVENT_RANGE_HIGHER       = 4,
    EVENT_RANGE_LOWER        = 5,
    EVENT_DISTANCE           = 6,
    EVENT_DISTANCE_LEFT      = 7,
    EVENT_DISTANCE_RIGHT     = 8,
};

struct Record
{
    uint8_t key[2];
    uint8_t end_event;
    uint8_t event_param[2];

    uint16_t getBigNum() const { return ((event_param[0] << 8) | (event_param[1] & 0xFF)); }
    void setBigNum(uint16_t num)
    {
        event_param[0] = uint8_t(num >> 8);
        event_param[1] = uint8_t(num & 0xFF);
    }
};

/*enum rangeMethods 
{
    RANGE_INCHES      = 0x50,
    RANGE_CENTIMETRES = 0x51,
    RANGE_MS          = 0x52,

    RANGE_INCHES_FAKE = 0x56,
    RANGE_CENTIMETRES_FAKE=0x57,
    RANGE_MS_FAKE     = 0x58,
};*/

enum states
{
    STATE_CORRECTION  = 0x01,
    STATE_RECORD      = 0x02,
    STATE_PLAY        = 0x04,
    STATE_ERASING     = 0x08,
    STATE_CORRECTION2 = 0x10,
};
