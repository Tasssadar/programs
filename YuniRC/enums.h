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
#define MEM_SIZE 50
#define REC_SIZE 10 // sizeof(Record)

struct Record
{
    char key[7];
    uint16_t time;
    bool filled;
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
};
