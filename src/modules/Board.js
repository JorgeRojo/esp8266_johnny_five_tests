import { EtherPortClient } from 'etherport-client';
import five from 'johnny-five';
import * as config from './../config.json';

class Board {

    board = null;
    _onReadyEvents = [];

    constructor() {

        console.log(" Board constructor ");
        this.board = new five.Board({
            port: new EtherPortClient({
                host: config.board_host,
                port: config.board_port,
            }),
            repl: false
        });

        this.board.on('ready', () => {
            this._onReadyEvents.forEach((event, index) => {
                event({
                    name: 'board_on_ready',
                    index,
                    board: this.board
                });
            });
        });
    }

    onReady = (event) => {
        if (typeof event == 'function') {
            this._onReadyEvents.push(event);
        }
    }

}

export default new Board();