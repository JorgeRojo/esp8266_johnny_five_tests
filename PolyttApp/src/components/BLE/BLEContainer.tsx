import React, { PureComponent } from "react";

import { BleManager, Device } from 'react-native-ble-plx';
import { Platform } from "react-native";
import utf8 from 'utf8';
import base64 from 'base-64';

import BLE from "./BLE";
import LoadSpinner from "../LoadSpinner";

const SERVICE_UUID = "91e88e4d-66b6-40b7-aa14-d5af542a7f0b";
const CHARACTERISTIC_UUID = "19a09ba4-51f4-45eb-a2d9-bec08dad539e";


class BLEContainer extends PureComponent {

    manager: BleManager;
    device: Device | null;
    isConnected: Boolean = false;

    state = {
        pass: '',
        wifi: '',
        wifiList: [],
    }

    constructor(props: any) {
        super(props);
        this.manager = new BleManager();
        this.device = null;
    }

    info = (str: String) => {
        console.info('BLE INFO: ', str);
    }

    error = (str: String) => {
        console.warn('BLE ERROR! ', str);
    }

    setPass = (pass: String) => {
        this.setState({ pass });
    }

    setWifi = (itemValue: any) => {
        this.setState({ wifi: itemValue }); 
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

    msgMemo = {
        msgStart: false,
        msg: '',
    };
    constructMsg = (chunk: string) => {  
        if (chunk === '<BLEMSG>') {
            this.msgMemo.msgStart = true;
            this.msgMemo.msg = '';
        }
        else if (chunk === '</BLEMSG>') {
            this.parseMsg(this.msgMemo.msg);
            this.msgMemo.msgStart = false;
            this.msgMemo.msg = '';
        }
        else if (this.msgMemo.msgStart) {
            this.msgMemo.msg += chunk;
        }
    }
    parseMsg = (rawMsg: string) => {
        try { 
            const wifiList = JSON.parse(rawMsg);
            this.setState({ wifiList });
        } catch (e) {
            this.error(rawMsg);
        }
    }

    scanAndConnect() {

        this.manager.startDeviceScan(null, null, (error, device) => {
            this.info(`scanning... ${device && device.name}`);

            if (error) {
                this.error(error.message);
                this.setState({ wifiList: [] });
                return;
            }

            if (device && device.name && device.name.match(/^TopDone-/g)) {
                this.info(`connecting... ${device.name}`);
                this.manager.stopDeviceScan();

                device
                    .connect()
                    .then(device => {
                        this.device = device;
                        this.info("discovering...");
                        return device.discoverAllServicesAndCharacteristics();
                    })
                    .then(device => {

                        this.info("connected...");
                        this.isConnected = true;
                        this.setState({ wifiList: [] });

                        //TODO: RECEIVE
                        device.monitorCharacteristicForService(
                            SERVICE_UUID,
                            CHARACTERISTIC_UUID,
                            (error, characteristic) => {
                                if (error) {
                                    this.error(error.message);
                                    this.setState({ wifiList: [] });
                                    this.isConnected = false;
                                    device.cancelConnection();
                                    this.scanAndConnect();
                                    return;
                                }
                                if (characteristic && characteristic.value) {
                                    this.constructMsg(base64.decode(characteristic.value));
                                }

                            }
                        );

                    },
                        error => {
                            this.error(error.message);
                            this.setState({ wifiList: [] });
                            this.isConnected = false;
                            device.cancelConnection();
                            this.scanAndConnect();
                        }
                    );
            }
        });


    }

    sendBLEMessage = () => {
        if (this.device && this.isConnected) {

            const config: String = JSON.stringify({
                wifi_ssid: this.state.wifi,
                wifi_pass: this.state.pass
            });

            const size = 20;

            const msg: [String] = [];
            msg.push("<BLEMSG>".padEnd(size, " ")); 
            for (let i = 0; i < config.length; i += size) {
                const chunk = config.slice(i, i + size);
                msg.push(chunk.padEnd(size, " ")); 
            }
            msg.push("</BLEMSG>".padEnd(size, " "));

            msg.forEach((chunk: String) => { 
                this.device.writeCharacteristicWithoutResponseForService(
                    SERVICE_UUID,
                    CHARACTERISTIC_UUID,
                    base64.encode(utf8.encode(chunk))
                );
            });
            
        }
    };

    componentWillMount() {
        this.startBLEScanner();
    }

    componentWillUpdate() {
        this.startBLEScanner();
    }

   

    render() {

        if(!this.state.wifiList || this.state.wifiList.length <= 0) {
            return (<LoadSpinner />);
        }

        return (<BLE 
            pass={this.state.pass}
            wifi={this.state.wifi}
            setWifi={this.setWifi}
            wifiList={this.state.wifiList}
            setPass={this.setPass}
            sendBLEMessage={this.sendBLEMessage}
        />);
    }
}

export default BLEContainer;
