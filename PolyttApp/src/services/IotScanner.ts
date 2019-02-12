import Wifi from "react-native-iot-wifi";
import { store } from "~/store";
import { setIotStatus } from "~/store/status/iot/actions";

class IotScanner   { 

    private ssid: string = '';

    async init() {

        this.ssid = '';

        const wifiAvailable = await this.isAvailable();
     

        if (wifiAvailable) {
            this.ssid = await this.getSSID();
        }    
        
        store.dispatch(setIotStatus({ 
            initialled: true,
            wifiAvailable,
            wifiError: !this.ssid,
            iotScanning: !!this.ssid,
        })); 

        if (this.ssid) { 
            this.scanIot() 
        }
 

    }


    private scanIot = async () => {
        let iotExsits = this.ssid;
        
        iotExsits = await this.connectSecure('sssadasdds', 'A28042804a*');
        console.warn('>>>>------------> iotExsits', iotExsits);
        iotExsits = await this.connectSecure('sssasdasdads', 'A28042804a*');
        console.warn('>>>>------------> iotExsits', iotExsits);
        iotExsits = await this.connectSecure('sssasdadads', 'A28042804a*');
        console.warn('>>>>------------> iotExsits', iotExsits);
        iotExsits = await this.connectSecure('sssasdsaads', 'A28042804a*');
        console.warn('>>>>------------> iotExsits', iotExsits);
    }
 
    private isAvailable = () => new Promise((resolve) => {
        Wifi.isAvaliable(resolve);
    })

    private getSSID = () => new Promise((resolve) => {
        Wifi.getSSID(resolve); 
    }) 
 
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

    private connectSecure = (ssid: string, password: string) => new Promise((resolve, reject) => {
        Wifi.connectSecure(ssid, password, false, (error: any) => {
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