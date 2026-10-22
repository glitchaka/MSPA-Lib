# MSPA-Lib

**MSPA — Motor de Simulación Psicológica y Cultural de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una librería conceptual para dotar agentes autónomos de personalidad, valores, metas, creencias subjetivas, emociones, ánimo, memoria, relaciones, aprendizaje y preferencias de acción explicables. También busca permitir que prácticas, normas y cultura emerjan de la interacción entre agentes, sin convertir “la cultura” en una entidad omnisciente separada.

> Estado actual: **diseño lógico / arquitectura candidata**. No existe implementación de producción todavía.

La librería está pensada para ser desacoplada del motor y del género de juego. No debe conocer sprites, pathfinding, inventarios concretos, escenas, animaciones, navegación, combate ni conceptos propios de un proyecto específico. El proyecto anfitrión describe lo que ocurre, qué puede percibirse y qué acciones son posibles; MSPA interpreta esos datos desde la perspectiva de cada agente y devuelve estado psicológico e intención explicable.

## Contrato conceptual

```text
HOST / MUNDO
    ↓ OBSERVE
estímulos percibidos
    ↓
MSPA actualiza estado interno
    ↓ DECIDE
opciones reales del host
    ↓
MSPA devuelve intención + expresión + explicación
    ↓
HOST ejecuta
    ↓ REPORT
resultado real
    ↓
MSPA aprende / actualiza
```

Además, `ADVANCE` comunica paso de tiempo lógico y `EXPORT / IMPORT` permite persistir o trasladar el estado psicológico de un agente entre runtimes o mundos.

## Principios de diseño

- El agente reacciona a **lo que cree**, no a la verdad global del juego.
- La percepción, la creencia, la inferencia y la reapreciación son procesos separados.
- Personalidad, valores, metas, emociones, ánimo, memoria y relaciones cumplen funciones distintas.
- La personalidad modifica sensibilidades y curvas de valoración; no entrega bonificaciones rígidas a acciones.
- Las creencias usan proposiciones semánticas con fuente y certeza.
- Las inferencias producen hipótesis mediante reglas declarativas y portables.
- El aprendizaje práctico se modela como expectativas `acción + contexto → resultado`.
- MSPA selecciona sólo entre acciones realmente ofrecidas por el host.
- Las relaciones son direccionales.
- Las emociones se representan como instancias con causa y objetivo.
- La cultura se entiende como distribución de creencias, valores, expectativas y prácticas entre agentes, no como un controlador global.
- El runtime debe ser explicable, reproducible y portable.
- El diseño debe escalar mediante eventos, evaluación perezosa y distintos niveles de simulación.
- Una misma librería debe poder funcionar con distintos juegos mediante un **Core Psychology Pack** y **Domain Packs** versionados.

## Estado del diseño

La versión vigente es una **arquitectura candidata v2.3**, todavía no congelada. Antes de implementar producción deben demostrarse en sandbox, entre otros:

- lenguaje de proposiciones;
- inferencia declarativa;
- composición de utilidad;
- aprendizaje causal y social;
- sesgos de actualización de creencias;
- histéresis;
- determinismo;
- migración de agentes entre mundos;
- divergencia y contacto cultural.

## Documentación vigente

- [`docs/MSPA-SPEC-v2.3.md`](docs/MSPA-SPEC-v2.3.md) — arquitectura lógica candidata completa.
- [`docs/MSPA-INTERACTION-CONTRACT-v0.1.md`](docs/MSPA-INTERACTION-CONTRACT-v0.1.md) — contrato neutral `OBSERVE → DECIDE → REPORT → ADVANCE`, tipos lógicos y portabilidad.
- [`docs/MSPA-SPEC-v2.1.md`](docs/MSPA-SPEC-v2.1.md) — especificación anterior conservada como historial de diseño.

## Fuera de alcance por ahora

No se ha autorizado todavía elegir lenguaje, ABI, bindings, sistema de build, formato de archivo ni serialización concreta. Tampoco se consideran demostradas todavía religiones emergentes, instituciones complejas, cultura interesante o innovación sostenida: son hipótesis que deberán validarse mediante simulación.
