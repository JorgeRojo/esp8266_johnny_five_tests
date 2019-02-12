export const SET_IOT_STATUS = 'SET_IOT_STATUS'; 

export interface IotStatus {
    initialled?: boolean,
    wifiAvailable?: boolean, 
    wifiError?: boolean,
    iotScanning?: boolean, 
    iotScanningError?: boolean, 
    iotConnected?: boolean, 
    iotWifiConnectingError?: boolean,
    iotWifiConnecting?: boolean,
}
  
interface SetIotStatusAction {
    type: typeof SET_IOT_STATUS
    payload: IotStatus
}
  
export type IotActionTypes = SetIotStatusAction;

