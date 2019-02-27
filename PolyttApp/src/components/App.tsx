//https://www.polidea.com/blog/ReactNative_and_Bluetooth_to_An_Other_level/
import React, {Component} from "react"; 
import { Provider } from "react-redux"; 
import { PersistGate } from "redux-persist/es/integration/react"; 
import { store, persistor } from "~/store"; 
import Home from "~/components/Home";
import LoadSpinner from "~/components/LoadSpinner"; 
import { Platform } from "react-native";


import { BleManager } from 'react-native-ble-plx'; 
const SERVICE_UUID = "91e88e4d-66b6-40b7-aa14-d5af542a7f0b";
const CHARACTERISTIC_UUID = "19a09ba4-51f4-45eb-a2d9-bec08dad539e";

 


export default class App extends Component {   

    manager: BleManager; 
   
    constructor(props: any) {
        super(props);
        this.manager = new BleManager();
    }

    info = (str: String) => {
        console.info('>>>>------------> str', str);
    }

    error = (str: String) => {
        console.error('>>>>------------> str', str);
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

                            console.warn('>>>>------------> characteristic', characteristic.value);
                        });

                    }, (error) => {
                        this.error(error.message)
                    })
            }
        });
    }
 

    render() {
        return ( 
            <Provider store={store}>
                <PersistGate loading={<LoadSpinner/>} persistor={persistor}> 
                    <Home/>
                </PersistGate>
            </Provider>
        );
    }
} 


