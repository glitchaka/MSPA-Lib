# MSPA — Especificación lógica v2.4

> Documento normativo, agnóstico al host. Sin ejemplos ni nombres de consumidores concretos.

## 1. Principio rector

MSPA mantiene e interpreta estado psicológico subjetivo. El host mantiene el mundo, percepción física, acciones disponibles y ejecución real. MSPA nunca inventa una acción física ausente de las opciones del host.

## 2. Identidad y aislamiento

Los IDs son fuertes. `AgentId = WorldId + local_id`. Varias instancias `Runtime` pueden coexistir sin estado mutable global obligatorio. RNG, agentes, registry y diagnostics pertenecen a cada runtime.

## 3. Tiempo

`PsychTime` es `uint64` lógico. Cada runtime posee un `TimeDomain` con escala racional. `process_time` es monotónico por agente; `event_time` puede referirse al pasado. Las importaciones entre escalas usan conversión racional y políticas explícitas.

## 4. Estado psicológico

`AgentPsychState` contiene personalidad, valores, creencias, emociones, mood, metas, memorias, relaciones, expectativas aprendidas, intención actual, RNG y tiempo procesado.

Personalidad: sociabilidad, estabilidad, empatía, persistencia, curiosidad y audacia. Valores expresan qué importa; personalidad modifica cómo el agente procesa evidencia, riesgo, continuidad y contexto.

## 5. Semántica

El conocimiento usa `Proposition` estructurada: predicado, roles, polaridad, namespace, calificadores, tiempo/procedencia opcionales. `PredicateSchema` declara roles, agencia, objetivos, consecuencias, relevancia, intencionalidad y conceptos relacionados.

El registry se publica mediante snapshots inmutables. Nuevas definiciones se construyen en staging y se publican entre batches.

## 6. Domain Packs

Los packs son datos externos versionados. Pueden definir vocabulario, predicates, schemas de acción, consideraciones/curvas, reglas de inferencia, tags y valores iniciales. Las dependencias incluyen versión mínima y se validan antes de aplicar el pack. Los packs no ejecutan código arbitrario ni elevan cuotas.

## 7. Creencias e inferencia

Las creencias conservan proposición, certeza, carga emocional, fuente y evidencia. Se actualizan según evidencia, confianza de fuente, rigidez, carga emocional y contradicción.

La inferencia es declarativa, acotada, no Turing-completa, multi-paso y limitada por profundidad, reglas e hipótesis. Produce hipótesis, no verdad global.

## 8. Appraisal y afecto

Appraisal considera relevancia, consecuencia, agencia, intencionalidad, certeza, control, norma, expectativa y relación. Emociones básicas: alegría, tristeza, miedo, ira, ansiedad y aversión. Mood mantiene valencia/activación separadas.

## 9. Metas, memoria y relaciones

Metas: activa, suspendida, bloqueada, completada o abandonada. Pocas dominan a la vez y la persistencia depende del agente.

Memoria conserva proposición, importancia, emoción, certeza, fuente, participantes, tiempo y reactivaciones. La consolidación es selectiva y acotada.

Relaciones son direccionales: afecto, confianza, respeto y familiaridad.

## 10. Aprendizaje

Primera forma causal:

```text
ActionType + ContextTags -> OutcomeTag
```

Mantiene intentos/ocurrencias, confianza y última activación. DECIDE puede usar estas expectativas.

## 11. Decisión

El host entrega `ActionOption` reales. Utility AI combina consideraciones relevantes: costes, riesgos, progreso, personalidad, necesidades genéricas, valores, metas, relación, emociones/mood, aprendizaje y configuraciones de Domain Pack.

`ActionContinuityKey` e histéresis reducen oscilación mediante progreso, coste de interrupción, persistencia, estabilidad del contexto y urgencia.

## 12. Operaciones

Lifecycle: `CREATE_AGENT`, `CREATE_AGENT_DERIVED`, `REMOVE_AGENT`, `HAS_AGENT`.

Simulación: `OBSERVE`, `DECIDE`, `REPORT`, `ADVANCE_TO`, wrapper `REACT`.

Persistencia: `EXPORT_AGENT`, `IMPORT_AGENT`, export/import binario.

Administración: staging/publish de registry y Domain Packs.

Batch: variantes de observe/decide/report/advance ordenadas por `(AgentId,input_sequence)` con conflictos explícitos.

## 13. Persistencia y portabilidad

El estado es portable y versionado. La serialización binaria valida magic, versiones, longitudes y cuotas. `RebaseRelative` convierte todos los timestamps psicológicos; `PreserveAbsolute` sólo acepta dominios compatibles. RNG usa `RekeyForDestination` por defecto en traslado y `PreserveState` para restore/replay explícito.

## 14. Determinismo

Objetivo lógico: mismo estado + registry + inputs + tiempo + seed producen el mismo resultado lógico. No se usa RNG global ni orden de hash/scheduling como desempate. No se promete todavía igualdad bit-a-bit entre toolchains distintos.

## 15. Seguridad y cuotas

Toda entrada externa es no confiable. `HardLimits` limita a `RuntimeLimits`; `DefaultLimits` llena defaults. Se acotan agentes, estado por agente, strings, proposiciones, aprendizaje, serialización y diagnostics. Malformaciones devuelven errores, no mutación silenciosa.

## 16. Errores

C++ usa `Result<T>`. Las fronteras públicas no dejan escapar excepciones. `bad_alloc` se transforma en `OutOfMemory`. La ABI C usa `mspa_status` y `mspa_last_error()`.

## 17. Explicabilidad

Diagnostics registra una ruta causal compacta desde observación y cambio de creencia hasta appraisal, candidatos de decisión, selección y aprendizaje. La instrumentación no modifica RNG ni decisiones.

## 18. Cultura e influencia

La cultura emerge de distribuciones de valores, normas percibidas, prácticas, expectativas y símbolos; no es un controlador global. Influencias externas pueden aceptarse, resistirse o reinterpretarse y no equivalen automáticamente a obediencia.

## 19. Fuera de alcance

MSPA no implementa física, navegación, animación, inventario, combate, mundo, scheduler del host ni adaptadores concretos. Esas responsabilidades permanecen fuera de la biblioteca.
