# MSPA-Lib

**MSPA — Motor de Simulación Psicológica y Cultural de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una librería para dotar agentes autónomos de personalidad, valores, metas, creencias subjetivas, emociones, ánimo, memoria, relaciones, aprendizaje y preferencias de acción explicables.

> Estado actual: **arquitectura lógica v2.4 + planificación de arquitectura de código v0.1**. La documentación vigente es completamente agnóstica al host.

MSPA no contiene lógica, nombres, clases, eventos, escenarios ni reglas de proyectos consumidores. Los hosts externos reciben los contratos y conectores públicos y realizan fuera de este repositorio el mapeo de sus propias entidades, eventos, acciones y conceptos.

## Contrato conceptual

```text
HOST
    ↓ OBSERVE
MSPA actualiza estado subjetivo
    ↓ DECIDE
MSPA selecciona entre opciones ofrecidas
    ↓
HOST ejecuta
    ↓ REPORT
MSPA aprende / actualiza
```

Además:
- todas las operaciones relevantes llevan tiempo lógico entero;
- `ADVANCE_TO` sirve para actualización temporal diferida;
- `EXPORT / IMPORT` permite persistencia y traslado de estado;
- existen operaciones batch con orden canónico y determinismo equivalente al procesamiento individual.

## Principios

- El agente reacciona a lo que cree, no a la verdad global.
- Percepción, creencia, inferencia y reapreciación son procesos separados.
- Las creencias usan proposiciones semánticas.
- MSPA define vocabulario base extensible mediante namespaces externos.
- Las inferencias usan reglas declarativas, acotadas y no Turing-completas.
- La personalidad modifica sensibilidades y curvas, no aplica bonificaciones rígidas.
- MSPA sólo selecciona entre acciones ofrecidas por el host.
- La histéresis usa `ActionContinuityKey` estable.
- El aprendizaje causal inicial se limita a `ActionType + ContextTags → OutcomeTags`.
- Las relaciones son direccionales.
- La cultura es una propiedad emergente de estados individuales, no un controlador global.
- Toda entrada externa se trata como no confiable.
- El runtime debe ser explicable, reproducible y portable.
- Los Domain Packs concretos pertenecen a proyectos consumidores externos.

## Candidatos técnicos

- **Producción:** C++ nativo.
- **Bindings:** fachada C opcional.
- **Tiempo contractual:** ticks lógicos enteros de 64 bits.
- **RNG:** independiente por agente.
- **Curvas configurables:** lineales por tramos como candidato inicial.

No están congelados todavía ABI, serialización, build system, threading model ni formato físico de Domain Packs.

## Documentación vigente

- [`docs/MSPA-SPEC-v2.4.md`](docs/MSPA-SPEC-v2.4.md) — especificación lógica actual.
- [`docs/MSPA-INTERACTION-CONTRACT-v0.2.md`](docs/MSPA-INTERACTION-CONTRACT-v0.2.md) — contrato de interacción.
- [`docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md`](docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md) — contrato de Domain Packs externos.
- [`docs/MSPA-CODE-ARCHITECTURE-v0.1.md`](docs/MSPA-CODE-ARCHITECTURE-v0.1.md) — planificación de arquitectura de código.

## Invariante de separación

**Ningún personaje, juego, repositorio, lugar, escenario de conversación o regla específica de un host pertenece al diseño normativo ni al core de MSPA.**

Los tests de conformidad usan exclusivamente casos abstractos y neutrales.