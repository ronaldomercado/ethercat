struct ENGINE_USER;

template <class T> struct ListNode
{
    ELLNODE node;
};

class ProcessDataObserver : public ListNode<ProcessDataObserver>
{
public:
    virtual void on_pdo_message(PDO_MESSAGE * message, int size) = 0;
    virtual ~ProcessDataObserver() {}
};

class XFCPort : public asynPortDriver
{
    int P_Missed;
public:
    void incMissed()
    {
        epicsInt32 value;
        getIntegerParam(P_Missed, &value);
        value = value + 1;
        if(value == INT32_MAX)
        {
            value = 0;
        }
        setIntegerParam(P_Missed, value);
        callParamCallbacks();
    }
    XFCPort(const char * name) : asynPortDriver(
        name,
        1, /* maxAddr */
        1, /* max parameters */
        asynInt32Mask | asynDrvUserMask, /* interface mask*/
        asynInt32Mask, /* interrupt mask */
        0, /* non-blocking, no addresses */
        1, /* autoconnect */
        0, /* default priority */
        0) /* default stack size */
    {
        createParam("MISSED", asynParamInt32, &P_Missed);
        setIntegerParam(P_Missed, 0);
    }
};

class ecAsyn : public asynPortDriver, public ProcessDataObserver
{
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    int pdos;
    int devid;
    rtMessageQueueId writeq;
    EC_PDO_ENTRY_MAPPING ** mappings;
    int P_AL_STATE;
#define FIRST_SLAVE_COMMAND P_AL_STATE
    int P_ERROR_FLAG;
    int P_DISABLE;
#define LAST_SLAVE_COMMAND P_DISABLE
    int P_First_PDO;
    int P_Last_PDO;
public:
    ecAsyn(EC_DEVICE * device, int pdos, ENGINE_USER * usr, int devid);
    EC_DEVICE * device;
    virtual void on_pdo_message(PDO_MESSAGE * message, int size);
    virtual asynStatus getBounds(asynUser *pasynUser, epicsInt32 *low, epicsInt32 *high);
};

#define NUM_SLAVE_PARAMS (&LAST_SLAVE_COMMAND - &FIRST_SLAVE_COMMAND + 1)

class ecMaster : public asynPortDriver, public ProcessDataObserver
{
    int P_Cycle;
#define FIRST_MASTER_COMMAND P_Cycle
    int P_WorkingCounter;
    int P_Missed;
    int P_WcState;
#define LAST_MASTER_COMMAND P_WcState
    epicsInt32 lastCycle;
    epicsInt32 missed;
public:
    ecMaster(char * name);
    virtual void on_pdo_message(PDO_MESSAGE * message, int size);
};

#define NUM_MASTER_PARAMS (&LAST_MASTER_COMMAND - &FIRST_MASTER_COMMAND + 1)
