import React, { Component } from "react";

import { View, TextInput, Text, StyleSheet } from "react-native";
import { Button } from 'react-native-elements';


export default class BLE extends Component<{
    ssid: String,
    pass: String,
    setPass: Function,
    sendBLEMessage: Function,
}> {
    render() {
        return (
            <View style={styles.wrapper}>

                <Text>founded Wifi SSID</Text>
                <Text style={styles.title}>{this.props.ssid}</Text>

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
    title: {
        fontSize: 18,
        marginBottom: 16,
    },
    input: {
        borderWidth: 1,
        marginBottom: 16,
    }
});