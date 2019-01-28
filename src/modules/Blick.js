
import Board from './Board'; 
import five from 'johnny-five';

const LED_PIN = 2;

class Blink {

    constructor() {
        this.init();
    }

    handleBroadOnReady = ({ board }) => {

        board.pinMode(LED_PIN, five.Pin.OUTPUT);
        // the Led class was acting hinky, so just using Pin here
        const pin = five.Pin(LED_PIN);
        let value = 0;
        setInterval(() => {
            if (value) {
                pin.high();
                value = 0;
            } else {
                pin.low();
                value = 1;
            }
        }, 100); 
    }

    init() {
        Board.onReady(this.handleBroadOnReady);
    }
}

export default new Blink();