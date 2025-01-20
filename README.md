# WHEN injectdll how to call SetWindowBand?  
1.after your inject,you should press WIN key to call `NtUserEnableIAMAccessHook`  
GET this urL:http://127.0.0.1:1145/call?HWND=***&HWND2=**&MODE=** to set zband  
replace *** to hwnd
# shutdown:http://127.0.0.1:1145/exit will kill explorer.exe and restart  
#MODE:
```C++
enum ZBID
{
    ZBID_DEFAULT = 0,
    ZBID_DESKTOP = 1,
    ZBID_UIACCESS = 2,
    ZBID_IMMERSIVE_IHM = 3,
    ZBID_IMMERSIVE_NOTIFICATION = 4,
    ZBID_IMMERSIVE_APPCHROME = 5,
    ZBID_IMMERSIVE_MOGO = 6,
    ZBID_IMMERSIVE_EDGY = 7,
    ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
    ZBID_IMMERSIVE_INACTIVEDOCK = 9,
    ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
    ZBID_IMMERSIVE_ACTIVEDOCK = 11,
    ZBID_IMMERSIVE_BACKGROUND = 12,
    ZBID_IMMERSIVE_SEARCH = 13,
    ZBID_GENUINE_WINDOWS = 14,
    ZBID_IMMERSIVE_RESTRICTED = 15,
    ZBID_SYSTEM_TOOLS = 16,
    //Windows 10+
    ZBID_LOCK = 17,
    ZBID_ABOVELOCK_UX = 18,
};
```
thanks to:https://gist.github.com/ADeltaX/80d220579400a95c336af0eec372ecb0 gice me soucre code  
thanks to Wormwaker([bilibili](https://space.bilibili.com/3494361276877525?spm_id_from=333.1387.follow.user_card.click)) let me know zband  
btw recommend o2 optimize
