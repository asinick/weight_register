import sqlite3

# Create a connection to the new SQLite database file
conn = sqlite3.connect("weight_data_new.db")
c = conn.cursor()

# Create a table for weight data in the new database if it doesn't exist
c.execute(
    """
    CREATE TABLE IF NOT EXISTS weight (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        mass INTEGER,
        datetime DATETIME,
        shift INTEGER
    )
    """
)

# Rest of the code...
