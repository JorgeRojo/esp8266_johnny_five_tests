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
    wifiConnected: false,
    wifiError: null,
    iotScanning: false,
    iotConnected: false,
    iotError: null,
};

const iotStatusPersistsConfig = {
    key: 'PolyttApp:IotStatus',
    storage: FilesystemStorage,
    whitelist: [
        'initialled',
    ],
    blacklist: [
        'wifiAvailable',
        'wifiConnected',
        'wifiError',
        'iotScanning',
        'iotConnected',
        'iotError',
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