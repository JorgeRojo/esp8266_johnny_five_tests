import { Board } from 'firmata';
import { EtherPortClient } from 'etherport-client'; 
import * as config from './../config.json';
 


const _onReadyEvents = [];
const board = new Board(new EtherPortClient({
    host: config.board_host,
    port: config.board_port
}));

board.on('ready', () => {
    _onReadyEvents.forEach((event, index) => { 
        console.log("--------- BOARD READY ---------");
        console.log(
            board.firmware.name + " " +
            board.firmware.version.major + "." +
            board.firmware.version.minor
        ); 
        console.log(board);
        console.log("-------------------------------");
        event({ index, board }); 
    });
}); 

export const onReady = (event) => {
    if (typeof event == 'function') {
        _onReadyEvents.push(event);
    }
} 

 