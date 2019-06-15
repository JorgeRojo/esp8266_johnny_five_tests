import React, { Component } from "react";

import { View, TextInput, Text, StyleSheet, Picker } from "react-native";
import { Button } from 'react-native-elements';


export default class BLE extends Component<{
    wifi: String,
    wifiList: any,
    pass: String,
    setWifi: Function,
    setPass: Function,
    sendBLEMessage: Function,
}> {
    render() {

        return (
            <View style={styles.wrapper}>

                <Text>founded Wifi SSID</Text> 

                <View style={styles.input}> 
                    <Picker style={[styles.input, { marginBottom: 0 }]}
                        selectedValue={this.props.wifi} 
                        onValueChange={this.props.setWifi}>
                            {this.props.wifiList.map(( item, i ) => ( 
                                <Picker.Item key={i} label={item} value={item} /> 
                            ))}
                    </Picker>
                </View>

                <Text>Tipe the Wifi password</Text>
                <TextInput style={styles.input}
                    onChangeText={(pass) => this.props.setPass(pass)}
                    value={this.props.pass}
                />

                <Button title='SEND' onPress={this.props.sendBLEMessage} />
            </View>
        );
    }
}

const styles = StyleSheet.create({
    wrapper: {
        width: '100%',
    }, 
    input: {
        borderWidth: 1,
        marginBottom: 16,
        borderColor: '#000'
    }
});