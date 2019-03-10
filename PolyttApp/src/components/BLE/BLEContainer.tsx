import React, { PureComponent } from "react";

import { BleManager, Device } from 'react-native-ble-plx';
import { Platform } from "react-native";
import utf8 from 'utf8';
import base64 from 'base-64';

import BLE from "./BLE";

const SERVICE_UUID = "91e88e4d-66b6-40b7-aa14-d5af542a7f0b";
const CHARACTERISTIC_UUID = "19a09ba4-51f4-45eb-a2d9-bec08dad539e";
const DEVICE_NAME = "Polytt_9R4W7bvff9";


class BLEContainer extends PureComponent<{
    ssid: String;
}> {

    manager: BleManager;
    device: Device | null;
    isConnected: Boolean = false;

    state = {
        pass: '',
    }

    constructor(props: any) {
        super(props);
        this.manager = new BleManager();
        this.device = null;
    }

    info = (str: String) => {
        console.warn('BLE INFO: ', str);
    }

    error = (str: String) => {
        console.warn('BLE ERROR! ', str);
    }

    setPass = ( pass: String) => {
        this.setState({ pass });
    }

    startBLEScanner = () => {
        if (this.device && this.isConnected) {
            return;
        }

        if (Platform.OS === "ios") {
            this.manager.onStateChange(state => {
                if (state === "PoweredOn") this.scanAndConnect();
            });
        } else {
            this.scanAndConnect();
        }
    }

    scanAndConnect() {

        this.manager.startDeviceScan(null, null, (error, device) => {
            this.info("scanning...");

            if (error) {
                this.error(error.message);
                return;
            }

            if (device && device.name === DEVICE_NAME) {
                this.info("connecting...");
                this.manager.stopDeviceScan();

                device
                    .connect()
                    .then(device => {
                        this.device = device;
                        this.info("discovering...");
                        return device.discoverAllServicesAndCharacteristics();
                    })
                    .then(
                        device => {
                            this.info("connected...");
                            this.isConnected = true;

                            //TODO: RECEIVE
                            return device.monitorCharacteristicForService(
                                SERVICE_UUID,
                                CHARACTERISTIC_UUID,
                                (error, characteristic) => {
                                    if (error) {
                                        this.error(error.message);
                                        return;
                                    }
                                    if (characteristic && characteristic.value) {
                                        this.info(`${base64.decode(characteristic.value)}`);
                                    }
                                }
                            );
                        },
                        error => {
                            this.error(error.message);
                            this.isConnected = false;
                        }
                    );
            }
        });
    }

    sendBLEMessage = () => { 
        if (this.device && this.isConnected) {

            const config: Object = {
                wifi_ssid: this.props.ssid,
                wifi_pass: this.state.pass
            };

            const msg: String = "<MSG>" + JSON.stringify(config) + "<#MSG>";

            for (let i = 0; i < Math.ceil(msg.length / 20); i++) {
                this.device.writeCharacteristicWithoutResponseForService(
                    SERVICE_UUID,
                    CHARACTERISTIC_UUID,
                    base64.encode(utf8.encode(msg.slice(i * 20, (i + 1) * 20)))
                );
            }
        }
    };

    componentWillMount() {
        this.startBLEScanner();  
    }

    componentWillUpdate() { 
        this.startBLEScanner(); 
    }

    render() {
        return (<BLE
            ssid={this.props.ssid}
            pass={this.state.pass}
            setPass={this.setPass}
            sendBLEMessage={this.sendBLEMessage}
        />);
    }
}

export default BLEContainer;
