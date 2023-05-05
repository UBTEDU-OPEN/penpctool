#ifndef IPCPROTOCOL_H
#define IPCPROTOCOL_H

#define AIPEN_SERVER_NAME "AiPenServer"

enum emLocalSocketIPCPro
{
    ePro_PowerMsg,
    ePro_FirmwareVersion,
    ePro_AttendClass,
    ePro_FinishClass,
    ePro_FinishAllClass,
    ePro_StartClasswork,
    ePro_StopClasswork,
    ePro_StartDictation,
    ePro_StopDictation,
    ePro_DictationWord,
    ePro_NoFn,

#ifdef SHOW_TEST_BTN
    ePro_OpenOriginFile = 200,
#endif
};


template <class T> struct LocalSocketMsg
{
    int type;
    T MsgPack;
};

#endif // IPCPROTOCOL_H
