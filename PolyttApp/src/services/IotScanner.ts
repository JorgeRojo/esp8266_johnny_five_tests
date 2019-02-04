import Wifi from "react-native-iot-wifi";
import { store } from "~/store";
import { setIotStatus } from "~/store/status/iot/actions";

class IotScanner   { 

    private ssid: string = '';

    async init() { 
        
        this.ssid = '';
        store.dispatch(setIotStatus({ initialled: true }));

        const wifiAvailable = await this.isAvailable();
        store.dispatch(setIotStatus({ wifiAvailable }));

        if(wifiAvailable) { 
            this.ssid = await this.getSSID();
        }

        if(!this.ssid) { 
            store.dispatch(setIotStatus({ wifiError: true}));  
        }
        else {
            store.dispatch(setIotStatus({ wifiConnected: true}));
            //TODO... continue flow -> scan iots
        }      

        store.dispatch(setIotStatus({ initialled: false }));

    }
       
    private isAvailable = () => new Promise((resolve) => {
        Wifi.isAvaliable(resolve);
    })

    private getSSID = () => new Promise((resolve) => {
        Wifi.getSSID(resolve); 
    })

    private scanIots = () => { 
    }
 
    private connect = (ssid) => new Promise((resolve, reject) => { 
        Wifi.connect(ssid, (error) => {
            if(error) {
                reject(error);
            }
            else {
                Wifi.getSSID(resolve);
            }
        }); 
    })

    private connectSecure = (ssid, password) => new Promise((resolve, reject) => {
        Wifi.connectSecure(ssid, password, (error) => {
            if(error) {
                reject(error);
            }
            else {
                Wifi.getSSID(resolve);
            }
        }); 
    })

    private removeSSID = (ssid) => new Promise((resolve) => {
        Wifi.removeSSID(ssid, resolve); 
    }) 

} 

export default new IotScanner();