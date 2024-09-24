package main

import (
	"fmt"
	"log"
	"net/http"

	"github.com/gorilla/websocket"
	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // Allow all origins
	},
}

func main() {
	// Initialize WebSocket server
	http.HandleFunc("/ws", handleWebSocket)

	// Start WebSocket server
	fmt.Println("WebSocket server listening on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}

// WebSocket handler
func handleWebSocket(w http.ResponseWriter, r *http.Request) {
	// Upgrade the HTTP connection to a WebSocket connection
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("Upgrade error:", err)
		return
	}
	defer conn.Close()

	// Open the serial port
	port, err := openSerialPort("/dev/cu.usbmodemDC5475C560D42", 19200)
	if err != nil {
		log.Println("Error opening serial port:", err)
		return
	}
	defer port.Close()

	// Listen for messages from the WebSocket
	for {
		_, message, err := conn.ReadMessage()
		if err != nil {
			log.Println("Read error:", err)
			break
		}
		fmt.Printf("Received message: %s\n", message)

		// Write the message to the serial port
		_, err = port.Write(message)
		if err != nil {
			log.Println("Error writing to serial port:", err)
			break
		}
	}
}

// Oopen and configure the serial port
func openSerialPort(portName string, baudRate int) (serial.Port, error) {
	mode := &serial.Mode{
		BaudRate: baudRate,
		Parity:   serial.NoParity,
		DataBits: 8,
		StopBits: serial.OneStopBit,
	}

	// Check available ports
	ports, err := enumerator.GetDetailedPortsList()
	if err != nil {
		return nil, fmt.Errorf("failed to list serial ports: %v", err)
	}

	for _, p := range ports {
		fmt.Printf("Found ports: %s\n", p.Name)
	}

	port, err := serial.Open(portName, mode)
	if err != nil {
		return nil, fmt.Errorf("failed to open serial port: %v", err)
	}
	return port, nil
}
