//https://www.polidea.com/blog/ReactNative_and_Bluetooth_to_An_Other_level/
//https://polidea.github.io/react-native-ble-plx/
import React, {Component} from "react"; 
import { Provider } from "react-redux"; 
import { PersistGate } from "redux-persist/es/integration/react"; 
import { store, persistor } from "~/store"; 
import Home from "~/components/Home";
import LoadSpinner from "~/components/LoadSpinner"; 
import { Platform, TouchableWithoutFeedback, Text, StyleSheet } from "react-native"; 


import randomSentence from 'random-sentence'; 
import utf8 from 'utf8'; 
import base64 from 'base-64'; 
import { BleManager, Device } from 'react-native-ble-plx'; 
const SERVICE_UUID = "91e88e4d-66b6-40b7-aa14-d5af542a7f0b";
const CHARACTERISTIC_UUID = "19a09ba4-51f4-45eb-a2d9-bec08dad539e";

 


export default class App extends Component {   

    manager: BleManager; 
    device: Device | null;
   
    constructor(props: any) {
        super(props);
        this.manager = new BleManager();
        this.device = null;
    }

    info = (str: String) => {
        console.info('>>>>------------> BLE', str);
    }

    error = (str: String) => {
        console.error('>>>>------------> BLE', str);
    }

    componentWillMount() {
        if (Platform.OS === 'ios') {
            this.manager.onStateChange((state) => {
                if (state === 'PoweredOn') this.scanAndConnect();
            });
        } else {
            this.scanAndConnect();
        }
    } 
  
    scanAndConnect() {
        this.manager.startDeviceScan(null, null, (error, device) => {

            this.info("Scanning...");


            if (error) {
                this.error(error.message)
                return
            }

            if (device && (device.name === 'Polytt config')) {
                this.info("Connecting to TI Sensor")
                this.manager.stopDeviceScan()
                device.connect()
                    .then((device) => {
                        this.device = device;
                        this.info("Discovering services and characteristics")
                        return device.discoverAllServicesAndCharacteristics()
                    }) 
                    .then((device) => {
                        this.info("Listening...");
 

                        return device.monitorCharacteristicForService(SERVICE_UUID, CHARACTERISTIC_UUID, (error, characteristic) => {
                            if (error) {
                                this.error(error.message)
                                return
                            }

                            this.info(`PING -> ${base64.decode(characteristic && characteristic.value)}`); 
 
                        });

                    }, (error) => {
                        this.error(error.message)
                    })
            }
        });
    }

    sendMessage = () => {  
        const text = randomSentence({words: 5});

        if(this.device) {
            this.device.writeCharacteristicWithoutResponseForService(SERVICE_UUID, CHARACTERISTIC_UUID, base64.encode(utf8.encode(text)));
        } 
    }
 

    render() {
        return ( 
            <Provider store={store}>
                <PersistGate loading={<LoadSpinner/>} persistor={persistor}> 
                    <TouchableWithoutFeedback onPress={this.sendMessage}><Text style={styles.text}>TEST</Text></TouchableWithoutFeedback>
                    {/* <Home /> */}
                </PersistGate>
            </Provider>
        );
    }
} 




const styles = StyleSheet.create({ 
    text: {
        fontSize: 20,
        textAlign: 'center',
        margin: 10,
    }, 
});