import React, {Component} from "react";     
import {StyleSheet, Text, View} from 'react-native';  
import { Interface } from "readline";
  
export default class Home extends Component  {  

    get text () { 
        const {iot} = this.props; 

        if(iot.initialled && !iot.wifiAvailable) {
            return 'Wifi service is not available, you need a device with wifi connection';
        }

        if(iot.initialled && iot.wifiError) {
            return 'Wifi is not connected, you need wifi connection';
        }

        if(iot.initialled && !iot.wifiConnected) {
            return 'Wifi connecting ...';
        }
         
        return `I need the password of your wifi to connect the iot device`; 
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