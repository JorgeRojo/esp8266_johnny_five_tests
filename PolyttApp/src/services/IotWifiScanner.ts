import Wifi from "react-native-iot-wifi";

class IotWifiScanner   { 
      
    init = async () => { 
        const avaliable =  await this.isAvaliable();
        if(avaliable) {   
            const ssid = await this.getSSID();
            if(ssid !== '<unknown ssid>') {
                //TODO: continue flow
            } 
            else {
                console.warn('You need  are connected to xxx a wifi net'); 
            }  
        }  
        else {
            console.warn('wifi system not avaliable');
        }  
    } 

    isAvaliable = () => new Promise((resolve) => {
        Wifi.isAvaliable(resolve);
    })

    getSSID = () => new Promise((resolve) => {
        Wifi.getSSID(resolve); 
    })
 
    connect = (ssid) => new Promise((resolve, reject) => { 
        Wifi.connect(ssid, (error) => {
            if(error) {
                reject(error);
            }
            else {
                Wifi.getSSID(resolve);
            }
        }); 
    })

    connectSecure = (ssid, password) => new Promise((resolve, reject) => {
        Wifi.connectSecure(ssid, password, (error) => {
            if(error) {
                reject(error);
            }
            else {
                Wifi.getSSID(resolve);
            }
        }); 
    })

    removeSSID = (ssid) => new Promise((resolve) => {
        Wifi.removeSSID(ssid, resolve); 
    }) 

} 

export default new IotWifiScanner();