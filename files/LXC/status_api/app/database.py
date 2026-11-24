import sqlite3

def init_db():
    conn = sqlite3.connect("status.db")
    cursor = conn.cursor()
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS geraete_status (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_name TEXT UNIQUE NOT NULL,
            is_on BOOLEAN NOT NULL,
            is_requested BOOLEAN NOT NULL,
            is_verified BOOLEAN NOT NULL
        )
    """)
    conn.commit()
    conn.close()

def get_status(device_name: str):
    conn = sqlite3.connect("status.db")
    cursor = conn.cursor()
    cursor.execute("SELECT is_on, is_verified FROM geraete_status WHERE device_name = ?", (device_name,))
    result = cursor.fetchone()
    cursor.execute("""
        UPDATE geraete_status 
        SET is_requested = 1 
        WHERE device_name = ?
    """, (device_name,))
    conn.commit()
    conn.close()

    if result:
        return {"is_on": bool(result[0]), "is_requested": bool(True), "is_verified": bool(result[1])}
    return None

def get_all_status():
    conn = sqlite3.connect("status.db")
    cursor = conn.cursor()
    cursor.execute("SELECT device_name, is_on, is_requested, is_verified FROM geraete_status")
    rows = cursor.fetchall()
    conn.close()

    return [{"device_name": r[0], "is_on": bool(r[1]), "is_requested": bool(r[2]), "is_verified": bool(r[3])} for r in rows]

def add_device(device_name: str, is_on: bool):
    conn = sqlite3.connect("status.db")
    cursor = conn.cursor()
    cursor.execute("""
        INSERT INTO geraete_status (device_name, is_on, is_requested, is_verified)
        VALUES (?, ?, 1, 1)
        ON CONFLICT(device_name) DO NOTHING
    """, (device_name, int(is_on)))
    conn.commit()
    conn.close()

def update_is_verified(device_name: str):
    conn = sqlite3.connect("status.db")
    cursor = conn.cursor()
    cursor.execute("""
        UPDATE geraete_status
        SET is_verified = 1 
        WHERE device_name = ?
    """, (device_name,))
    conn.commit()
    conn.close()

def update_is_on(device_name: str, is_on:bool):
    conn = sqlite3.connect("status.db")
    cursor = conn.cursor()
    cursor.execute("""
        UPDATE geraete_status
        SET is_on = ?,  is_verified = 1
        WHERE device_name = ?
    """, (int(is_on), device_name))
    conn.commit()
    conn.close()
