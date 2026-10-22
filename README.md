# MSPA-Lib

**MSPA — Motor de Simulación Psicológica y Cultural de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una librería para dotar agentes autónomos de personalidad, valores, metas, creencias subjetivas, emociones, ánimo, memoria, relaciones, aprendizaje y preferencias de acción explicables.

> Estado actual: **arquitectura lógica v2.4 + arquitectura de código v0.1 + contrato de interacción v0.2 + implementación inicial de `base` en C++17**.

MSPA es completamente agnóstico al host. No contiene lógica, nombres, clases, eventos, escenarios ni reglas de proyectos consumidores. Los hosts externos reciben los contratos/conectores públicos y realizan fuera de este repositorio el mapeo de sus entidades, eventos, acciones y conceptos.

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
- `CREATE_AGENT / REMOVE_AGENT / HAS_AGENT` gestionan ciclo de vida;
- todas las operaciones relevantes usan `PsychTime` entero de 64 bits;
- `process_time` y `event_time` son conceptos distintos;
- `ADVANCE_TO` sirve para actualización temporal diferida;
- `EXPORT_AGENT / IMPORT_AGENT` permiten persistencia y traslado de estado;
- existen operaciones batch con orden canónico y determinismo equivalente al procesamiento individual;
- varias instancias `Runtime` pueden coexistir en el mismo proceso sin estado mutable global compartido.

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

## Decisiones técnicas actuales

- **Producción:** C++17 nativo.
- **Bindings:** fachada C opcional.
- **Identidad:** `AgentId = WorldId + local_id`.
- **Errores C++:** `Result<T>`; excepciones no atraviesan API pública.
- **Errores C:** `mspa_status` + parámetros de salida.
- **Tiempo contractual:** ticks lógicos enteros de 64 bits y escala racional por runtime.
- **RNG:** independiente por agente; rekey por defecto al trasladar entre runtimes.
- **Batch:** snapshot semántico inmutable durante la operación; orden `(AgentId, input_sequence)`; mutaciones se publican entre batches.
- **Curvas configurables:** lineales por tramos como formato inicial.
- **Compilación determinista recomendada:** MSVC `/fp:strict`; GCC/Clang sin `-ffast-math` y con contracción FP desactivada cuando sea relevante.

No están congelados todavía el formato físico de serialización, el modelo de threading interno, la representación numérica para garantía bit-a-bit multiplataforma ni el layout exacto de la ABI C.

## Implementación actual

El primer bloque ejecutable vive en:

```text
include/mspa/base/
src/base/
tests/base_tests.cpp
```

Actualmente cubre:
- IDs fuertes;
- `AgentId` compuesto;
- `PsychTime`, `TimeScale` y `TimeDomain`;
- `ImportTimePolicy`;
- `ErrorCode` y `Result<T>`;
- RNG determinista por agente y `RngImportPolicy`;
- build CMake C++17;
- primer test unitario/conformidad de `base`.

## Documentación vigente

- [`docs/MSPA-SPEC-v2.4.md`](docs/MSPA-SPEC-v2.4.md) — especificación lógica actual.
- [`docs/MSPA-INTERACTION-CONTRACT-v0.2.md`](docs/MSPA-INTERACTION-CONTRACT-v0.2.md) — contrato de interacción vigente y corregido en sitio.
- [`docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md`](docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md) — contrato de Domain Packs externos.
- [`docs/MSPA-CODE-ARCHITECTURE-v0.1.md`](docs/MSPA-CODE-ARCHITECTURE-v0.1.md) — arquitectura de código vigente y corregida en sitio.

## Invariante de separación

**Ningún personaje, juego, repositorio, lugar, escenario de conversación o regla específica de un host pertenece al diseño normativo ni al core de MSPA.**

Los tests de conformidad usan exclusivamente casos abstractos y neutrales.
