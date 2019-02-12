import FilesystemStorage from 'redux-persist-filesystem-storage'; 
import { persistReducer } from 'redux-persist';

import {
    IotStatus,
    IotActionTypes,
    SET_IOT_STATUS,
} from './types';
 
const initialState: IotStatus = {
    initialled: false,
    wifiAvailable: false, 
    wifiError: false,
    iotScanning: false, 
    iotScanningError: false, 
    iotConnected: false, 
    iotWifiConnectingError: false,
    iotWifiConnecting: false,
};

const iotStatusPersistsConfig = {
    key: 'PolyttApp:IotStatus',
    storage: FilesystemStorage,
    whitelist: [
    ],
    blacklist: [ 
        'initialled',
        'wifiAvailable', 
        'wifiError',
        'iotScanning', 
        'iotScanningError', 
        'iotConnected', 
        'iotWifiConnectingError',
        'iotWifiConnecting',
    ],
};

function iotReducer(state = initialState, { type, payload }: IotActionTypes): IotStatus {
    switch (type) {
        case SET_IOT_STATUS:
            return {
                ...state,
                ...payload
            }
        default:
            return state;
    }
}

export default persistReducer(iotStatusPersistsConfig, iotReducer); 