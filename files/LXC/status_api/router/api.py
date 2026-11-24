from fastapi import APIRouter
from app.database import get_status, add_device, update_is_verified, update_is_on, get_all_status

router = APIRouter(prefix="/api")

@router.get("/get_status/{device_name}")
def api_get_status(device_name: str):
    status_data = get_status(device_name)
    if status_data is None:
        return {"error": "Bezeichner nicht gefunden"}
    return {"device_name": device_name, "status": status_data}

@router.post("/update_is_verified/{device_name}")
def api_update_is_verified(device_name: str):
    update_is_verified(device_name)
    return {"message": f"Status fuer {device_name} validiert"}

@router.post("/add_device/{device_name}/{is_on}")
def api_add_device(device_name: str, is_on: bool):
    add_device(device_name, is_on)
    return {"message": f"Device {device_name} wurde hinzugefuegt"}

@router.post("/update_is_on/{device_name}/{is_on}")
def api_update_is_on(device_name: str, is_on: bool):
    update_is_on(device_name, is_on)
    return {"message": f"Device {device_name} wurde aktualisiert"}

@router.get("/get_all_status")
def api_get_all_status():
    devices = get_all_status()
    return {"devices": devices}
