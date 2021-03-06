<h1>Συσκευή μέτρησης ποιοτικών χαρακτηριστικών του αέρα</h1>
<img src="images/airquality.jpg" alt="Συσκευή μέτρησης ποιοτικών χαρακτηριστικών του αέρα">
<p>Η συσκευή έχει τη δυνατότητα να πραγματοποιεί μετρήσεις της ποιότητας του αέρα. Πιο συγκεκριμένα, μπορεί να μετρήσει τα παρακάτω:</p>
<ul>
    <li>Θερμοκρασία - Υγρασία</li>
    <li>Συγκέντρωση αερίων (CO, NO2)</li>
    <li>Συγκέντρωση sωματιδίων μεγέθους 1.0, 2.5 και 10 μm</li>
</ul>

<p>Οι μετρήσεις αποθηκεύονται σε μια βάση δεδομένων και προβάλλονται με διάφορους τύπους γραφημάτων. Η αποστολή των μετρήσεων, από τους αισθητήρες προς τη Βάση Δεδομένων, γίνεται μέσω του πρωτοκόλλου MQTT.</p>

<h2>Υλικά</h2>
<p>Τα υλικά που χρησιμοποιήθηκαν, για την κατασκευή του μετρητή, είναι τα παρακάτω:</p>
<ul>
    <li><a href="https://store.arduino.cc/arduino-mega-2560-rev3" target="_blank">Arduino Mega 2560</a></li>
    <li><a href="https://wiki.seeedstudio.com/Ethernet_Shield_V2.0/" target="_blank">Grove Ethernet Shield</a></li>
    <li><a href="https://wiki.seeedstudio.com/Grove-Temperature_Humidity_Pressure_Gas_Sensor_BME680/" target="_blank">Grove Temperature Humidity Sensor</a></li>
    <li><a href="https://wiki.seeedstudio.com/Grove-Multichannel_Gas_Sensor/" target="_blank">Grove Multichannel Gas Sensor</a></li>
    <li><a href="https://wiki.seeedstudio.com/Grove-I2C_Hub/" target="_blank">Grove I2C Hub</li>
    <li><a href="https://www.raspberrypi.org/products/raspberry-pi-3-model-b/" target="_blank">Raspberry Pi 3B</a></li>
</ul>

<h2>Λογισμικό</h2>
<p>Εγκαταστάθηκε και παραμετροποιήθηκε το παρακάτω λογισμικό, στο Raspberry Pi 3B:</p>
<ul>
    <li><a href="https://www.influxdata.com/" target="_blank">InfluxDB</a></li>
    <li><a href="https://nodered.org/" target="_blank">Node-RED</a></li>
    <li><a href="https://mosquitto.org/" target="_blank">Eclipse Mosquitto</a></li>
    <li><a href="https://grafana.com/" target="_blank">Grafana</a></li>
</ul>

<p>Για εύκολη και γρήγορη εγκατάσταση-παραμετροποίηση των παραπάνω λογισμικών, μπορεί να χρησιμοποιηθεί το <a href="https://github.com/gcgarner/IOTstack">IOTstack</a></p>
