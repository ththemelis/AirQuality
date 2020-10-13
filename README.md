# AirQuality
Συσκευή μέτρησης ποιοτικών χαρακτηριστικών του αέρα. Πιο συγκεκριμένα γίνεται μέτρηση:
* Θερμοκρασίας
* Υγρασίας
* Αερίων (CO, NO2)
* Σωματιδίων μεγέθους 1.0, 2.5 και 10 μm

Οι μετρήσεις αποθηκεύονται σε μια βάση δεδομένων (InfluxDB) και προβάλλονται με διάφορους τύπους γραφημάτων (Grafana). Η αποστολή των μετρήσεων γίνεται μέσω MQTT. Αυτά τα λογισμικά εκτελούνται σε ένα Raspberry Pi 3B.

## Σύνδεση


### Βιβλιοθήκη
[Βιβλιοθήκη](https://github.com/JChristensen/DS3232RTC)

## Αισθητήρας θερμοκρασίας

