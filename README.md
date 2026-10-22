# MSPA-Lib

**MSPA — Motor de Simulación Psicológica y Cultural de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una librería conceptual para dotar agentes autónomos de personalidad, valores, metas, creencias subjetivas, emociones, ánimo, memoria, relaciones, aprendizaje y preferencias de acción explicables. También busca permitir que prácticas, normas y cultura puedan emerger de la interacción entre agentes sin convertir “la cultura” en una entidad omnisciente separada.

> Estado actual: **diseño lógico / arquitectura candidata v2.4**. No existe implementación de producción todavía.

La librería está pensada para ser desacoplada del motor y del género de juego. El host describe qué ocurre, qué puede percibirse, qué acciones son realmente posibles y cuál fue su resultado. MSPA interpreta esos datos desde la perspectiva subjetiva de cada agente y devuelve estado psicológico, intención y explicación.

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

Además:

- todas las operaciones relevantes llevan tiempo lógico;
- `ADVANCE_TO` existe para agentes dormidos, hitos y actualización temporal explícita, no como tick obligatorio;
- `EXPORT / IMPORT` permite persistir o trasladar estado psicológico entre runtimes o mundos;
- existen formas batch conceptuales con orden canónico y determinismo equivalente al procesamiento individual.

## Principios de diseño

- El agente reacciona a **lo que cree**, no a la verdad global del juego.
- Percepción, creencia, inferencia y reapreciación son procesos separados.
- Las creencias se expresan mediante proposiciones semánticas.
- MSPA define un vocabulario base y los juegos lo extienden mediante namespaces.
- Las inferencias usan reglas declarativas, acotadas, portables y no Turing-completas.
- La personalidad modifica sensibilidades y curvas; no entrega bonificaciones rígidas a acciones.
- MSPA selecciona sólo entre acciones ofrecidas por el host.
- La histéresis usa una identidad estable de continuidad, no IDs efímeros.
- `OBSERVE` puede recomendar una reevaluación urgente sin tomar la decisión por el host.
- El aprendizaje causal inicial se limita a `ActionType + ContextTags → OutcomeTags`.
- Las relaciones son direccionales.
- La cultura se interpreta como distribución de creencias, valores, expectativas y prácticas entre agentes.
- El runtime debe ser explicable, reproducible, portable y resistente a datos externos no confiables.
- Los estados y packs importados deben respetar límites de tamaño y complejidad.
- El diseño debe escalar mediante evaluación perezosa, niveles de simulación y batching determinista.
- La integración de cada juego se describe mediante un **Domain Pack** versionado.

## Arquitectura vigente

La versión actual es **MSPA v2.4 — arquitectura candidata**. Todavía no está congelada.

Antes de fijar una API de producción deben validarse en sandbox:

- lenguaje de proposiciones;
- vocabulario base;
- inferencia declarativa acotada;
- composición de utility AI por consideraciones y curvas;
- continuidad e histéresis;
- retorno de urgencia de `OBSERVE`;
- aprendizaje causal simple;
- determinismo individual y batch;
- migración de agentes entre mundos;
- importación segura de estados y packs;
- divergencia y contacto cultural;
- un Domain Pack realista de Isla Viva.

## Domain Packs

Un juego no modifica el core para enseñarle sus conceptos. Entrega un Domain Pack con:

- vocabulario propio;
- Predicate Schemas;
- Event Templates;
- Action Schemas;
- reglas de inferencia;
- consideraciones y curvas;
- ContextTags y OutcomeTags de aprendizaje;
- compatibilidad y namespaces;
- límites de seguridad.

MSPA debe incluir además un **Core Psychology Pack** con semántica psicológica general.

## Candidatos técnicos

Todavía **no se ha autorizado implementación**.

Como candidatos actuales:

- **Sandbox:** Python, con escenarios neutrales reutilizables después como suite de conformidad.
- **Producción:** Rust con ABI C estable como frontera de integración.

Estas elecciones siguen siendo candidatas y deben validarse contra el sandbox y el `IslaVivaPsychPack`. No se ha congelado ABI, serialización, build system ni bindings.

## Documentación vigente

- [`docs/MSPA-SPEC-v2.4.md`](docs/MSPA-SPEC-v2.4.md) — arquitectura lógica candidata actual.
- [`docs/MSPA-INTERACTION-CONTRACT-v0.2.md`](docs/MSPA-INTERACTION-CONTRACT-v0.2.md) — contrato `OBSERVE / DECIDE / REPORT / ADVANCE_TO / EXPORT-IMPORT`.
- [`docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md`](docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md) — contrato de integración semántica por juego.

### Historial conservado

- [`docs/MSPA-SPEC-v2.3.md`](docs/MSPA-SPEC-v2.3.md)
- [`docs/MSPA-INTERACTION-CONTRACT-v0.1.md`](docs/MSPA-INTERACTION-CONTRACT-v0.1.md)
- [`docs/MSPA-SPEC-v2.1.md`](docs/MSPA-SPEC-v2.1.md)

## Próxima prueba de diseño

Antes de escribir el runtime de producción debe existir un **`IslaVivaPsychPack v0.1`** suficientemente completo para representar sin excepciones específicas dentro de MSPA:

- necesidades;
- familia;
- trabajo;
- propiedad;
- nacimiento y muerte;
- duelo;
- visitantes y viajeros;
- aprendizaje;
- transmisión de creencias;
- influencia divina;
- choque de normas entre comunidades.

Si Isla Viva exige excepciones internas en el core para expresar estos casos, el contrato todavía no está listo.
