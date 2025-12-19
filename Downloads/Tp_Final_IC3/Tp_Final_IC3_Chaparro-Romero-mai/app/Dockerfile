FROM python:3.9-slim

WORKDIR /app

# Copiar librerías e instalarlas
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copiar los scripts
COPY . .

# Comando por defecto (se sobrescribe luego)
CMD ["python", "suscriptor.py"]