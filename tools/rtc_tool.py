import serial
import time
import argparse

def send_unix_timestamp(serial_port):
    try:
        # Abre el puerto serie
        with serial.Serial(serial_port, 9600, timeout=1) as ser:
            print(f"Conectado al puerto {serial_port} a 9600 baudios")

            # Obtiene el timestamp Unix actual
            unix_timestamp = int(time.time())
            print(f"Timestamp Unix actual: {unix_timestamp}")  # Depuración: imprime el timestamp actual

            # Formatea el mensaje con la cabecera 'T'
            message = f"T{unix_timestamp}\r\n"  # Incluye \r\n para asegurar compatibilidad

            # Agregar un retraso antes de enviar
            time.sleep(1)

            # Envía el mensaje al Arduino
            ser.write(message.encode('utf-8'))
            print(f"Enviado: {message.strip()}")

            # Agregar un retraso para asegurar que el Arduino procese el mensaje
            time.sleep(1)

            # Lee la respuesta del Arduino (opcional)
            response = ser.readline().decode('utf-8').strip()
            if response:
                print(f"Respuesta del Arduino: {response}")

    except serial.SerialException as e:
        print(f"Error al abrir el puerto serie: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Envía el timestamp Unix actual al puerto serie especificado.")
    parser.add_argument("serial_port", help="Nombre del puerto COM al que está conectado el Arduino (ej. COM3 o /dev/ttyUSB0).")
    args = parser.parse_args()

    send_unix_timestamp(args.serial_port)
