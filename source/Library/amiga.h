/* AmigaOS includes */

#ifndef _AMIGA_H
#define _AMIGA_H

#include <dopus/common.h>

#ifdef __amigaos4__

	// for os4 replace external (non L_) calls on internal (L_ ones)
	#define GetSemaphore L_GetSemaphore
	#define FreeSemaphore L_FreeSemaphore
	#define GetDosPathList L_GetDosPathList
	#define FindPubScreen L_FindPubScreen
	#define AllocMemH L_AllocMemH
	#define FreeMemH L_FreeMemH
	#define AllocTimer L_AllocTimer
	#define StartTimer L_StartTimer
	#define CheckTimer L_CheckTimer
	#define FreeTimer L_FreeTimer
	#define IPC_Reply L_IPC_Reply
	#define IPC_Free L_IPC_Free
	#define IPC_Command L_IPC_Command
	#define GetString L_GetString
	#define SetWindowID L_SetWindowID
	#define DrawBox L_DrawBox
	#define Att_NewList L_Att_NewList
	#define Att_RemList L_Att_RemList
	#define Att_NewNode L_Att_NewNode
	#define Att_RemList L_Att_RemList
	#define OpenClipBoard L_OpenClipBoard
	#define ReadClipString L_ReadClipString
	#define WriteClipString L_WriteClipString
	#define CloseClipBoard L_CloseClipBoard
	#define CopyLocalEnv L_CopyLocalEnv
	#define GetDeviceUnit L_GetDeviceUnit
	#define BuildKeyString L_BuildKeyString
	#define DeviceFromHandler(par1, par2) L_DeviceFromHandler(par1, par2, dopuslibbase_global)
	#define WB_Install_Patch() L_WB_Install_Patch(dopuslibbase_global)
	#define UpdateMyPaths() L_UpdateMyPaths(dopuslibbase_global)
	#define SendNotifyMsg(par1, par2, par3, par4, par5, par6) \
		L_SendNotifyMsg(par1, par2, par3, par4, par5, par6, dopuslibbase_global)
	#define DoSimpleRequest(par1, par2) L_DoSimpleRequest(par1, par2, dopuslibbase_global)

#endif

#endif /* _AMIGA_H */
