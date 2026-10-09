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
5. **Gestos y Controles en Pantalla de Bloqueo**:
   - **Doble Tap en pantalla para Luz Frontal**: Puedes encender o apagar la luz frontal rápidamente haciendo doble toque táctil en la pantalla sin necesidad de desbloquear el dispositivo.
   - **Botón Home en Pantalla de Bloqueo**: Opción en **Ajustes → Controles → Botón Inicio → En pantalla de bloqueo**. Permite usar el botón de inicio físico/capacitivo directamente en la pantalla de bloqueo para las acciones seguras autorizadas (**Luz frontal** y **Modo oscuro**), ya sea configuradas en toque simple, doble toque o pulsación larga.
   - **Protección contra Bypass de Seguridad**: Cualquier otra acción que intente navegar (como *Volver/Inicio*, *Menú del lector*, marcadores, etc.) queda estrictamente deshabilitada en la pantalla de bloqueo para evitar accesos no autorizados.
   - **Indicador `(solo desbloqueado)` / `(only unlocked)`**: En el selector de acciones del botón de inicio en Ajustes, cuando la opción "En pantalla de bloqueo" está activada, todas las funciones restringidas muestran claramente esta advertencia.
6. **Personalización del Título**:
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

### ⭐ Método Recomendado: Actualización desde Tarjeta SD (SD Card Firmware Update)

Si tu dispositivo ya tiene instalado **CrossInk** o **CrossPoint**, este es el método más rápido, seguro y limpio:

1. Descarga el archivo de release **[`releases/crossink-passcode-1.0.bin`](./releases/crossink-passcode-1.0.bin)** (o desde la sección [Releases de GitHub](https://github.com/nicovfilms-ux/crossink-passcode-xteink-x4pro/releases)).
2. Conecta la tarjeta MicroSD a tu PC (o usa el modo `Home → File Transfer → USB Drive` en el X4 Pro) y copia el archivo `crossink-passcode-1.0.bin` en cualquier lugar de la tarjeta SD (por ejemplo, en la raíz o en una carpeta de descargas).
3. En tu lector, ve a **Settings → System → SD Card Firmware Update** (o *Ajustes → Sistema → Actualizar Firmware desde SD*).
4. Navega hasta el archivo `.bin`, selecciónalo y confirma la actualización. El dispositivo validará la imagen, la grabará en la partición OTA correspondiente y se reiniciará con **CrossInk Passcode 1.0**.

---

### ⚠️ ¿Dispositivo de fábrica con Firmware Chino o Puerto USB bloqueado? (Flujo Unlocker / Escape Hatch)

Muchos dispositivos Xteink X4 Pro comprados a distribuidores de AliExpress vienen de fábrica con el firmware de stock chino o con el puerto USB bloqueado para flasheo directo, impidiendo instalar firmwares de la comunidad mediante el navegador web o cable USB común.

Si este es tu caso, sigue este procedimiento probado de desbloqueo:

#### Paso 1: Desbloqueo inicial (Web Unlocker o Escape Hatch)
- **Opción A (Unlocker Web):** Conecta el dispositivo por WiFi y utiliza la herramienta de desbloqueo oficial de la comunidad CrossPoint ([crosspointreader.com](https://crosspointreader.com/)) para liberar las particiones OTA.
- **Opción B (Escape Hatch Firmware vía SD - Recomendado si falla el Unlocker):**
  1. Si la herramienta web arroja errores de conexión (ej. *Network unstable 9*) o el puerto USB no responde, descarga el firmware de recuperación **Escape Hatch** (`escape-hatch.bin`).
  2. Renombra el archivo a `update.bin` y cópialo en la raíz de una tarjeta MicroSD formateada en FAT32.
  3. Inserta la MicroSD en el Xteink y arráncalo en modo recuperación (manteniendo pulsado el botón de actualización o combinación de encendido).
  4. El firmware minimalista **Escape Hatch** arrancará en pantalla, permitiendo puentear el bloqueo chino e instalar una versión base de CrossPoint / CrossInk directamente desde la tarjeta SD.

#### Paso 2: Flashear firmware base funcional
- Desde el menú de **Escape Hatch**, selecciona flashear un firmware base de CrossPoint / CrossInk en la partición inactiva y activa el cambio de slot de arranque.
- Al reiniciar, tu dispositivo ya contará con el bootloader abierto y la interfaz del sistema operativo libre.

#### Paso 3: Actualizar a CrossInk Passcode 1.0
- Una vez dentro del sistema con el bootloader desbloqueado, copia **`crossink-passcode-1.0.bin`** a la tarjeta SD.
- Dirígete a **Settings → System → SD Card Firmware Update**, selecciona el binario y listo: tendrás la versión con bloqueo por PIN, pantalla de Lost & Found y todas las optimizaciones activadas.

---

### Métodos Alternativos (Flasheo Directo por Cable)

#### Vía Herramienta Web (Dispositivos con USB desbloqueado)
1. Conecta tu Xteink X4 Pro a la computadora vía USB-C en modo bootloader (mantén presionado el botón de arranque al conectar).
2. Abre [CrossPoint Flash Tools](https://crosspointreader.com/#flash-tools) o [ESP Web Flasher](https://espressif.github.io/esptool-js/).
3. Selecciona el archivo binario **`crossink-passcode-1.0.bin`** y flashea en el offset de aplicación `0x10000`.

#### Vía esptool (Línea de Comandos)
```bash
esptool.py --chip esp32s3 --port COM_TU_PUERTO -b 921600 write_flash 0x10000 releases/crossink-passcode-1.0.bin
```

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
