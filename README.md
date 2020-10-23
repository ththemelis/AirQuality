# AirQuality
Συσκευή μέτρησης ποιοτικών χαρακτηριστικών του αέρα. Πιο συγκεκριμένα γίνεται μέτρηση:
* Θερμοκρασίας
* Υγρασίας
* Αερίων (CO, NO2)
* Σωματιδίων μεγέθους 1.0, 2.5 και 10 μm

Οι μετρήσεις αποθηκεύονται σε μια βάση δεδομένων (InfluxDB) και προβάλλονται με διάφορους τύπους γραφημάτων (Grafana). Η αποστολή των μετρήσεων, από τους αισθητήρες προς τη Βάση Δεδομένων, γίνεται μέσω MQTT.

<h2>Υλικά</h2>
<p>Τα υλικά που χρησιμοποιήθηκαν, για την κατασκευή του μετρητή, είναι τα παρακάτω:</p>
* Arduino Mega 2560
* Grove Shield
* Grove Temperature Humidity sensor
* Raspberry Pi 3B

Για περισσότερες πληροφορίες http://pektpekas.duckdns.org