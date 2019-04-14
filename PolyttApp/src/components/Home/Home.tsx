import React, { Component } from 'react';
import { StyleSheet, Text, View } from 'react-native';
import { IotStatus } from 'src/store/status/iot/types';

import { HomeProps } from '.';
import BLE from '~/components/BLE';

export default class Home extends Component<HomeProps> {

    //TODO get ssid from src/services/IotScanner.ts
    render() {
        return (
            <View style={styles.wrapper}>
                <BLE ssid="AOCARALLO_2G" />
            </View>
        );
    }
}

const styles = StyleSheet.create({
    wrapper: {
        flex: 1,
        alignItems: 'center',
        backgroundColor: '#F5FCFF',
        padding: 32,
    },
    text: {
        fontSize: 20,
        textAlign: 'center',
        margin: 10,
    },
});