export const SET_IOT_STATUS = 'SET_IOT_STATUS'; 

export interface IotStatus {
    initialled?: boolean,
    wifiAvailable?: boolean,
    wifiConnected?: boolean, 
    wifiError?: boolean,
    iotScanning?: boolean, 
    iotConnected?: boolean,
    iotError?: boolean,
}
  
interface SetIotStatusAction {
    type: typeof SET_IOT_STATUS
    payload: IotStatus
}
  
export type IotActionTypes = SetIotStatusAction;

