import React, {Component} from 'react';       
import {StyleSheet, Text, View} from 'react-native';   
import { IotStatus } from 'src/store/status/iot/types';
 
import { HomeProps } from '.';
  
export default class Home extends Component<HomeProps> {  

    get text() {
        const { iot } = this.props;

        if ( !iot.initialled ) {
            return `Starting...`; 
        }

        if (!iot.wifiAvailable) {
            return 'Wifi service is not available, you need a device with wifi connection';
        }

        if (iot.wifiError) {
            return 'Wifi is not connected, you need wifi connection';
        } 

        if (iot.iotScanning) {
            return `IOT devices scanning ...`;
        }

        if (iot.iotScanningError) {
            return `I can't found any iot device close`;
        }

        if (!iot.iotConnected) {
            return `I need the password of your wifi to connect the iot device`;
        } 

        if (iot.iotWifiConnectingError) {
            return `May be your wifi password is wrong, re-fill and try again`;
        } 

        if (iot.iotWifiConnecting) {
            return `Connecting iot to wifi ...`;
        } 
 
        return `You can configure your iot device 💚 `;
    }

    render() {
        return (   
            <View style={styles.wrapper}>
                <Text style={styles.text}>
                    { this.text  }                
                </Text> 
            </View>  
        );
    }
} 

const styles = StyleSheet.create({
    wrapper: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#F5FCFF',
    },
    text: {
        fontSize: 20,
        textAlign: 'center',
        margin: 10,
    }, 
});