/*
  XADOpus.module 1.22 - DOpus Magellan plugin to browse through XAD archives
  Copyright (C) 1999,2000 Mladen Milinkovic <mladen.milinkovic@ri.tel.hr>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "XADopus.h"

ULONG LIBFUNC L_ProgressHook(REG(a0, struct Hook *hook), REG(a2, int skip), REG(a1, struct xadProgressInfo *xpi))
{
	struct xoData *data = hook->h_Data;
	/*struct Library *DOpusBase;
	struct DosLibrary *DOSBase;
	struct UtilityBase *UtilityBase;*/
	//	struct TagItem tags[] = {{PW_FileDone, 0, TAG_DONE}};
	struct TagItem tags[] = {{PW_FileDone, 0}, {TAG_DONE}};
	/*	struct TagItem reqtags[]={AR_Window,0,AR_Message,0, AR_Button,0,
						AR_Button,0, AR_Button,0, AR_Button,0,
						AR_Button,0, AR_Button,0, TAG_DONE};*/
	ULONG ret = XADPIF_OK;
	LONG rc;
	char mess[200];
	DOpusCallbackInfo *infoptr = &data->hook;

	tags[0].ti_Data = xpi->xpi_CurrentSize;

	/*if(!(DOpusBase=data->DOpusBase)) return(0);
	if(!(DOSBase=data->DOSBase)) return(0);
	if(!(UtilityBase=data->UtilityBase)) return(0);*/

	SetProgressWindow(data->ptr, tags);

	switch (xpi->xpi_Mode)
	{
	case XADPMODE_ERROR: {
		sprintf(data->buf,
				"lister request %s \"%s\" %s|%s",
				data->lists,
				xadGetErrorText(xpi->xpi_Error),
				DOpusGetString(data->locale, MSG_OK),
				DOpusGetString(data->locale, MSG_ABORT));
		if (!DC_CALL4(infoptr,
					  dc_SendCommand,
					  DC_REGA0,
					  IPCDATA(data->ipc),
					  DC_REGA1,
					  data->buf,
					  DC_REGA2,
					  NULL,
					  DC_REGD0,
					  0))
			// if(!data->hook.dc_SendCommand(IPCDATA(data->ipc), data->buf, NULL, NULL))
			ret &= ~XADPIF_OK;
		break;
	}

	case XADPMODE_ASK: {
		if (xpi->xpi_Status & XADPIF_OVERWRITE)
		{
			if (data->All == TRUE)
			{
				ret |= XADPIF_OVERWRITE;
				break;
			}
			else if (data->All == FALSE)
				break;

			sprintf(mess, DOpusGetString(data->locale, MSG_EXISTS_FORM), FilePart(xpi->xpi_FileName));
			/*				reqtags[0].ti_Data = (ULONG) data->listw;
							reqtags[1].ti_Data = (ULONG) mess;
							reqtags[2].ti_Data = (ULONG) DOpusGetString(data->locale,MSG_REPLACE);
							reqtags[3].ti_Data = (ULONG) DOpusGetString(data->locale,MSG_REPLACE_ALL);
							reqtags[4].ti_Data = (ULONG) DOpusGetString(data->locale,MSG_SKIP);
							reqtags[5].ti_Data = (ULONG) DOpusGetString(data->locale,MSG_SKIP_ALL);
							reqtags[6].ti_Data = (ULONG) DOpusGetString(data->locale,MSG_ABORT);
							AsyncRequest(IPCDATA(data->ipc),REQTYPE_SIMPLE,NULL,NULL,NULL,reqtags);
			*/
			sprintf(data->buf,
					"lister request %s \"%s\" %s|%s|%s|%s|%s",
					data->lists,
					mess,
					DOpusGetString(data->locale, MSG_REPLACE),
					DOpusGetString(data->locale, MSG_REPLACE_ALL),
					DOpusGetString(data->locale, MSG_SKIP),
					DOpusGetString(data->locale, MSG_SKIP_ALL),
					DOpusGetString(data->locale, MSG_ABORT));

			rc = DC_CALL4(infoptr,
						  dc_SendCommand,
						  DC_REGA0,
						  IPCDATA(data->ipc),
						  DC_REGA1,
						  data->buf,
						  DC_REGA2,
						  NULL,
						  DC_REGD0,
						  0);
			// rc = data->hook.dc_SendCommand(IPCDATA(data->ipc), data->buf, NULL, NULL);

			if (!rc)
				ret = 0;
			else
			{
				if (rc <= 2)
					ret |= XADPIF_OVERWRITE;
				if (rc == 2)
					data->All = TRUE;
				else if (rc == 4)
					data->All = FALSE;
			}
		}
		break;
	}
	}

	return ret;
}
