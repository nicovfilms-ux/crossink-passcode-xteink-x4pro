# CrossInk Passcode Edition (Xteink X4 Pro)

[![Release](https://img.shields.io/badge/release-v1.0--passcode-blue.svg)](https://github.com/nicovfilms-ux/crossink-passcode-xteink-x4pro/releases)
[![Device](https://img.shields.io/badge/tested%20on-Xteink%20X4%20Pro-success.svg)](https://github.com/nicovfilms-ux/crossink-passcode-xteink-x4pro)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

> [!WARNING]
> **Compatibilidad de hardware:** Este firmware ha sido compilado, ajustado y probado minuciosamente en el **Xteink X4 Pro**. No se garantiza el funcionamiento en otras variantes de Xteink (X3, X4 estándar, X4 Classic) ni otros dispositivos ESP32 e-ink. Si decides flashearlo en otro hardware, **hazlo con cautela y bajo tu propio riesgo**.

---

## 📖 Descripción del Proyecto

**CrossInk Passcode Edition** es una evolución optimizada de [CrossInk](https://github.com/uxjulia/CrossInk) / [CrossInk-Bookorbit](https://github.com/agosez/CrossInk-Bookorbit) que incorpora una **capa de seguridad nativa con bloqueo por código PIN**, tarjeta de propietario / recuperación ("Lost & Found"), optimizaciones en el consumo y tiempos de suspensión, así como mejoras visuales y de rendimiento en la navegación de carátulas (Lyra Theme).

El archivo binario listo para instalación comunitaria es:
👉 **`crossink-passcode-1.0.bin`** (disponible en la carpeta [`releases/`](./releases/crossink-passcode-1.0.bin) y en la sección de [Releases de GitHub](https://github.com/nicovfilms-ux/crossink-passcode-xteink-x4pro/releases)).

---

## 🔒 Capa de Seguridad (PIN & Lost & Found)

Esta edición resuelve la ausencia de privacidad en los lectores electrónicos portátiles mediante un subsistema integrado de control de acceso:

1. **Bloqueo por PIN de 4 o 6 dígitos**:
   - Selector configurable de longitud de código PIN (4 o 6 dígitos) con renderizado dinámico de indicadores numéricos.
   - Teclado táctil en pantalla optimizado para tinta electrónica con retroalimentación instantánea, borrado directo (icono de retroceso integrado) y envío automático tras completar los dígitos.
2. **Disparadores de Bloqueo Inteligentes (Lock Triggers)**:
   - **Al suspender / apagar**: Bloqueo inmediato al entrar en reposo desde el botón de encendido o temporizador.
   - **Por inactividad**: Tiempo configurable independiente del sleep del lector (incluyendo opción de *Nunca* para uso continuo).
   - **Acceso rápido**: Activación bajo demanda desde el menú de ajustes del sistema.
3. **Flujo de Recuperación (Forgot PIN & Pregunta de Seguridad)**:
   - Pregunta secreta configurada por el usuario con respuesta cifrada/hasheada en almacenamiento no volátil para restablecer el PIN sin perder libros ni progreso.
4. **Tarjeta de Información del Propietario ("Lost & Found")**:
   - En la pantalla de bloqueo, se incluye un acceso directo de información (icono `ℹ`) que abre una tarjeta con los datos de contacto del dueño (Nombre y Teléfono con prefijo internacional automático `+`).
   - Permite que quien encuentre el dispositivo extraviado pueda contactarte sin tener acceso a tus libros ni datos privados.
5. **Personalización del Título**:
   - Nombre de usuario personalizado en el encabezado de bloqueo (`[Nombre] - XTEINK`).

---

## ⚡ Optimizaciones y Rendimiento

- **Suspensión Instantánea (Zero Lag Sleep)**:
  - Eliminación de popups bloqueantes transitorios antes de suspender (`Entering Sleep`), logrando que la pantalla de reposo se plasme de inmediato sin destellos dobles.
  - Tasa de refresco ajustada a `FAST_REFRESH` en la transición de pantallas de suspensión, reduciendo significativamente el parpadeo y el consumo energético.
- **Persistencia Atómica**:
  - Guardado seguro de credenciales y configuraciones en la partición NVS / SD card de forma atómica para prevenir corrupción de datos ante reinicios abruptos.
- **Gestión de Teclas Físicas**:
  - Detección precisa de pulsación corta y pulsación prolongada en el botón de encendido para diferenciar reposo inmediato de apagado completo.
  - Soporte de modo oscuro en la tecla Home y respuesta táctil acelerada.

---

## 🎨 Ajustes del Tema y Navegación (Lyra Theme)

- **Distribución Dinámica de Carátulas (Dynamic Cover Layout)**:
  - Rediseño del tema `Lyra 3 Covers` para adaptarse con precisión según la cantidad de libros recientes (1 a 6 libros).
  - Cálculo de proporciones de aspecto reales para evitar distorsiones en las portadas de los EPUBs.
  - Centrado balanceado de títulos y líneas de progreso de lectura (porcentaje y tiempo restante).
- **Navegación Matricial con Botones Físicos**:
  - Navegación bidireccional suave (Arriba, Abajo, Izquierda, Derecha) entre portadas y menú inferior, optimizada tanto para control táctil como por botones físicos en el X4 Pro.

---

## 🚀 Instalación en Xteink X4 Pro

### Opción 1: Mediante Herramienta Web (Recomendado)
1. Conecta tu Xteink X4 Pro a la computadora vía USB-C en modo flasheo.
2. Abre la herramienta web de flasheo ESP32 / CrossPoint Web Flasher ([crosspointreader.com](https://crosspointreader.com/#flash-tools) o [ESP Web Flasher](https://espressif.github.io/esptool-js/)).
3. Selecciona el archivo binario: **[`releases/crossink-passcode-1.0.bin`](./releases/crossink-passcode-1.0.bin)**.
4. Flashea en la dirección de memoria de aplicación estándar (`0x10000` según el esquema de partición).

### Opción 2: Mediante esptool (Línea de Comandos)
```bash
esptool.py --chip esp32s3 --port COM_TU_PUERTO write_flash 0x10000 releases/crossink-passcode-1.0.bin
```

### Opción 3: Actualización desde Tarjeta SD (si tu bootloader lo soporta)
1. Copia el archivo `crossink-passcode-1.0.bin` renombrado como `update.bin` o el nombre reconocido por tu bootloader en la raíz de la tarjeta MicroSD.
2. Enciende el dispositivo manteniendo presionado el botón asignado para actualización.

---

## 🛠️ Compilación desde el Código Fuente

El proyecto utiliza [PlatformIO](https://platformio.org/):

```bash
# Clonar el repositorio
git clone https://github.com/nicovfilms-ux/crossink-passcode-xteink-x4pro.git
cd crossink-passcode-xteink-x4pro

# Compilar para el entorno X4 Pro
pio run -e default

# Compilar y subir vía USB
pio run -e default --target upload
```

---

## ⚖️ Licencia y Créditos

Este proyecto está distribuido bajo la licencia [MIT](LICENSE). Basado en el excelente trabajo de la comunidad de [CrossInk](https://github.com/uxjulia/CrossInk) y [CrossInk-Bookorbit](https://github.com/agosez/CrossInk-Bookorbit).
