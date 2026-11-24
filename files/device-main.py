import RPi.GPIO as GPIO
import time
import requests
import os
from pathlib import Path
from urllib.parse import quote

def load_dotenv():
    script_directory = Path(__file__).resolve().parent
    dotenv_path = next(
        (directory / ".env" for directory in (script_directory, *script_directory.parents)
         if (directory / ".env").is_file()),
        None,
    )
    if dotenv_path is None:
        raise FileNotFoundError("Keine .env gefunden. Bitte .env.example nach .env kopieren.")

    for line_number, line in enumerate(dotenv_path.read_text(encoding="utf-8").splitlines(), start=1):
        entry = line.strip()
        if not entry or entry.startswith("#"):
            continue
        if entry.startswith("export "):
            entry = entry[len("export "):].lstrip()

        key, separator, value = entry.partition("=")
        key = key.strip()
        if not separator or not key.isidentifier():
            raise ValueError(f"Ungültiger Eintrag in .env, Zeile {line_number}")

        value = value.strip()
        if value[:1] in ("'", '"'):
            if len(value) < 2 or value[-1] != value[0]:
                raise ValueError(f"Nicht geschlossener Wert in .env, Zeile {line_number}")
            value = value[1:-1]
        else:
            value = value.split("#", 1)[0].strip()
        os.environ.setdefault(key, value)

def required_setting(name):
    value = os.getenv(name)
    if not value:
        raise ValueError(f"Die Einstellung {name} fehlt oder ist leer.")
    return value

def integer_setting(name):
    value = required_setting(name)
    try:
        return int(value)
    except ValueError as error:
        raise ValueError(f"Die Einstellung {name} muss eine Ganzzahl sein.") from error

load_dotenv()

API_BASE_URL = required_setting("API_BASE_URL").rstrip("/")
DEVICE_NAME = quote(required_setting("DEVICE_NAME"), safe="")
RELAY_PIN = integer_setting("RELAY_PIN")
LED_PIN = integer_setting("LED_PIN")
POLL_INTERVAL = integer_setting("POLL_INTERVAL")
API_URL_GET_DEV_STATUS = f"{API_BASE_URL}/get_status/{DEVICE_NAME}"
API_URL_SET_DEV_STATUS = f"{API_BASE_URL}/update_is_on/{DEVICE_NAME}"

# GPIO Setup
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

# Relais startet LOW
GPIO.setup(RELAY_PIN, GPIO.OUT, initial=GPIO.LOW)

# LED Eingang
GPIO.setup(LED_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

# global Var
last_pc_status_phy = None

def get_pc_status_phy():
    return GPIO.input(LED_PIN) == GPIO.HIGH

def pulse_power_button():
    print("Triggere Relais", flush=True)
    GPIO.output(RELAY_PIN, GPIO.HIGH)
    time.sleep(0.5)
    GPIO.output(RELAY_PIN, GPIO.LOW)
    print("Impuls beendet.", flush=True)

def report_status_to_api(is_on):
    try:
        url = f"{API_URL_SET_DEV_STATUS}/{str(is_on).lower()}"
        requests.post(url, timeout=2)
        print(f"\n[API-Sync] Realen PC-Status ({is_on}) an Webseite gemeldet.", flush=True)
    except Exception as e:
        print(f"\n[API-Sync Fehler] Konnte Status nicht melden: {e}", flush=True)

def check_hardware_integrity():
    global last_pc_status_phy
    if get_pc_status_phy() != last_pc_status_phy:
        raise HardwareChangeEvent()

print(f"Intelligenter PC-Controller gestartet.", flush=True)

try:
    # Intialer Zustand
    last_pc_status_phy = get_pc_status_phy()
    while True:
        try:

            check_hardware_integrity()

            r = requests.get(API_URL_GET_DEV_STATUS, timeout=3)
            status_dict = r.json().get("status", {})
            pc_status_api = status_dict.get("is_on")

            if pc_status_api and not last_pc_status_phy:
                print("API forder 'AN' aber PC ist 'AUS'.")
                pulse_power_button()
                last_pc_status_phy = True
                time.sleep(5)

            elif not pc_status_api and last_pc_status_phy:
                print("Api fordert 'AUS' aber PC ist 'AN'.")
                pulse_power_button()
                last_pc_status_phy = False
                time.sleep(5)

            else:
                pass

            for _ in range(POLL_INTERVAL):
                check_hardware_integrity()
                time.sleep(1)

        except HardwareChangeEvent:
            pc_status_phy = get_pc_status_phy()
            print(f"\n[Hardware-Interrupt] Manuelle Änderung erkannt! PC ist jetzt {'AN' if pc_status_phy else 'AUS'}.", flush=True)

            report_status_to_api(pc_status_phy)

            last_pc_status_phy = pc_status_phy
            print("[Sicherheit] Warte 5 Sekunden damit alle Prozesse abgeschlossen sind.", flush=True)
            time.sleep(5)
        except requests.exceptions.TImeout:
            print("\n[Timeout] Server nicht erreichbar. Keine Aktion durchgeführt.", flush=True)
        except Exception as e:
            print(f"Netwerk/API Fehler: {e}", flush=True)

except KeyboardInterrupt:
    print("Dienst beendet.")
finally:
    GPIO.cleanup()