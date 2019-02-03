import Wifi from "react-native-iot-wifi";

// Wifi.isAvaliable((avaliable) => {
//     console.warn(avaliable ? 'avaliable' : 'failed');
// });
   
// Wifi.getSSID((SSID) => {
//   console.warn(SSID);
// });

// Wifi.connect("wifi-name", (error) => {
//   console.warn(error ? 'error: ' + error : 'connected to wifi-name');
// });

// Wifi.removeSSID("wifi-name", (error)=>{
//   console.warn(error ? 'error: ' + error : 'removed wifi-name');
// });

class IotWifiScanner   { 
      
    init = async () => { 
        const avaliable =  await this.isAvaliable();
        if(avaliable) {  

            const ssid = await this.getSSID();
            if(ssid !== '<unknown ssid>') {
                //TODO: continue flow

            } 
            else {
                console.warn('You need  are connected to a wifi net'); 
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
 
    connect = (wifiName) => new Promise((resolve) => {
        Wifi.connect(wifiName, resolve); 
    })

    removeSSID = (wifiName) => new Promise((resolve) => {
        Wifi.removeSSID(wifiName, resolve); 
    }) 

} 

export default new IotWifiScanner();