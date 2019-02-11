import { IotStatus, SET_IOT_STATUS } from "./types";

export function setIotStatus(payload: IotStatus) {
    return {
        type: SET_IOT_STATUS,
        payload,
    }
} 
 