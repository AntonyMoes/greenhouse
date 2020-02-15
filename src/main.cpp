#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
public:
    explicit TemperatureSensor(unsigned int pin) : one_wire(pin), sensors(&one_wire) {
        sensors.begin();
    }

    float get_temperature() {
        // call sensors.requestTemperatures() to issue a global temperature
        // request to all devices on the bus
        sensors.requestTemperatures();

        // Why "byIndex"?
        // You can have more than one DS18B20 on the same bus.
        // 0 refers to the first IC on the wire
        return sensors.getTempCByIndex(0);
    }

private:
    OneWire one_wire;
    DallasTemperature sensors;
};

class GateManipulator {
public:
    GateManipulator(unsigned int gate_open_pin, unsigned int gate_close_pin) : curr_status(close), open_pin(gate_open_pin), close_pin(gate_close_pin) {
        pinMode(open_pin, OUTPUT);
        pinMode(close_pin, OUTPUT);
        set_gate(curr_status);
    }

    enum Status {open, close};

    Status get_status() {
        return curr_status;
    }

    void set_gate(Status status) {
        switch (status) {
            case open:
                open_gate();
                break;
            case close:
                close_gate();
                break;
        }
    }

    void close_gate() {
        transit_power(open_pin, close_pin);
        curr_status = close;
    }

    void open_gate() {
        transit_power(close_pin, open_pin);
        curr_status = open;
    }

    void switch_gate() {
        switch (curr_status) {
            case open:
                close_gate();
                break;
            case close:
                open_gate();
                break;
        }
    }

    const char* get_str_status() {
        switch (curr_status) {
            case open:
                return "open";
            case close:
                return "close";
            default:
                // enums are better in Rust (pattern matching and stuff)
                return "";  // this should never be reached, written to supress warnings
        }
    }

private:
    Status curr_status;
    const unsigned int open_pin;
    const unsigned int close_pin;

    static void transit_power(unsigned int pin1, unsigned int pin2) {
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
    }
};


const unsigned int TEMP_PIN = 2;
const unsigned int GATE_OPEN_PIN = 4;
const unsigned int GATE_CLOSE_PIN = 5;


TemperatureSensor sensor(TEMP_PIN);
GateManipulator gate_manipulator(GATE_OPEN_PIN, GATE_CLOSE_PIN);

void setup(void) {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void) {
    Serial.println("Requesting temperatures...");
    auto temp = sensor.get_temperature();

    if (temp < 30 && gate_manipulator.get_status() == GateManipulator::open) {
        gate_manipulator.close_gate();
    } else if (temp > 32 && gate_manipulator.get_status() == GateManipulator::close) {
        gate_manipulator.open_gate();
    }

    Serial.print("Gate status: ");
    Serial.println(gate_manipulator.get_str_status());

    Serial.print("Temperature is: ");
    Serial.println(temp);
    Serial.println();
    delay(1000);
}