from flask import Flask, render_template, jsonify
import serial
import serial.tools.list_ports
import threading
import json
import time

app = Flask(__name__)

telemetry = {
    "connected": False,
    "line": "00000",
    "action": "WAITING",
    "distance_cm": 0,
    "obstacle": False,
    "junction": False,
    "path_detected": False
}


def find_esp32():
    ports = serial.tools.list_ports.comports()

    for port in ports:
        description = (port.description or "").lower()

        if (
            "usb" in description
            or "uart" in description
            or "cp210" in description
            or "ch340" in description
        ):
            return port.device

    return None


def serial_reader():
    global telemetry

    while True:
        port = find_esp32()

        if port is None:
            telemetry["connected"] = False
            time.sleep(2)
            continue

        try:
            print(f"Connecting to ESP32 on {port}")

            ser = serial.Serial(
                port,
                115200,
                timeout=1
            )

            telemetry["connected"] = True

            while True:
                line = ser.readline().decode("utf-8", errors="ignore").strip()

                if not line:
                    continue

                try:
                    data = json.loads(line)

                    telemetry.update(data)
                    telemetry["connected"] = True

                except json.JSONDecodeError:
                    print("Serial:", line)

        except Exception as error:
            print("Serial connection error:", error)
            telemetry["connected"] = False
            time.sleep(2)


@app.route("/")
def home():
    return render_template("index.html")


@app.route("/api/telemetry")
def get_telemetry():
    return jsonify(telemetry)


if __name__ == "__main__":
    thread = threading.Thread(
        target=serial_reader,
        daemon=True
    )

    thread.start()

    print("AMCP Cargo OS")
    print("Dashboard: http://127.0.0.1:8000")

    app.run(
        host="127.0.0.1",
        port=8000,
        debug=True,
        use_reloader=False
    )
