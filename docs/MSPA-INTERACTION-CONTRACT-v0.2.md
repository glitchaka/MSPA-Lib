# MSPA Interaction Contract v0.2

> Contrato lógico candidato, agnóstico al host.

Este documento fija qué intercambian el host y MSPA y qué garantías lógicas deben conservarse entre implementaciones.

## 1. Operaciones conceptuales

### Ciclo de vida
- `CREATE_AGENT`
- `REMOVE_AGENT`
- `HAS_AGENT`

### Simulación
- `OBSERVE`
- `DECIDE`
- `REPORT`
- `ADVANCE_TO`

### Persistencia/traslado
- `EXPORT_AGENT`
- `IMPORT_AGENT`

### Administración semántica
- validar/cargar pack;
- construir staging registry;
- publicar registry snapshot entre batches.

`REACT` puede existir sólo como wrapper de `OBSERVE + DECIDE`.

## 2. Runtime e identidad

Cada instancia `Runtime` es independiente y posee su propio estado, agentes, seed, configuración, límites, diagnósticos, dominio temporal y snapshot semántico.

No existe estado mutable global obligatorio. Varias instancias pueden coexistir en un mismo proceso.

`AgentId` incluye el mundo de origen:

```text
AgentId
  WorldId world
  uint64 local_id
```

El mismo `local_id` en mundos distintos no representa al mismo agente.

## 3. Tiempo y dominios temporales

Toda operación simulacional relevante usa:

```text
PsychTime = uint64 logical ticks
```

Cada runtime posee un `TimeDomain` con escala racional e identidad estable. La escala se fija al crear el runtime y permanece estable durante su vida.

### Tiempo de procesamiento

Las llamadas simulacionales llevan `process_time`. Este tiempo debe ser monotónico para el estado del agente.

`process_time < LastProcessedTime` produce `TimeRegression`, salvo mecanismos explícitos futuros de replay/snapshot.

### Tiempo del evento

Un estímulo puede llevar `event_time` opcional. Representa cuándo ocurrió el hecho conocido, no cuándo lo procesa MSPA.

`event_time` puede ser anterior a `LastProcessedTime` y no produce por sí solo `TimeRegression`. Esto permite testimonios tardíos, mensajes de red retrasados, recuerdos y evidencia histórica.

## 4. CREATE_AGENT

Entrada conceptual:

```text
CREATE_AGENT(runtime, agent_id, agent_init, process_time)
```

`AgentInit` puede contener estado completo o parcial, personalidad, valores, relaciones, creencias/memorias iniciales autorizadas, seed y política de defaults.

Puede existir una política genérica de inicialización derivada desde uno o más estados fuente. No supone biología, parentesco ni dominio concreto.

Errores típicos:
- `DuplicateAgent`
- `InvalidArgument`
- `QuotaExceeded`
- `InvalidState`
- `OutOfMemory`

## 5. REMOVE_AGENT

Entrada:

```text
REMOVE_AGENT(runtime, agent_id, policy, process_time)
```

La eliminación del estado activo no obliga a borrar referencias históricas desde otros agentes. Éstas pueden conservarse como `EntityRef` no resolubles.

Errores típicos:
- `UnknownAgent`
- `InvalidArgument`

## 6. HAS_AGENT

Consulta read-only de existencia/registro. No altera simulación ni RNG.

## 7. OBSERVE

Entrada:

```text
OBSERVE(runtime, agent, stimulus, process_time)
```

`Stimulus` puede contener fuente, objetivo, proposiciones, confianza perceptiva, saliencia, namespace, `event_time` opcional y metadatos externos validados.

Salida:

```text
ObserveResult
  state_changed
  decision_recommended
  urgency
  current_action_threatened
  diagnostic_token?
```

`urgency`:
- `None`
- `Normal`
- `Soon`
- `Immediate`

MSPA recomienda scheduling; el host conserva el control.

## 8. DECIDE

Entrada:

```text
DECIDE(runtime, agent, available_actions, current_action?, process_time)
```

Cada `ActionOption` incluye al menos:
- `ActionTypeId`
- `ActionInstanceId`
- `ActionContinuityKey`
- actor
- objetivo(s)
- costes
- riesgos
- efectos esperados
- tags/contexto
- progreso opcional

Salida `Decision`:
- acción seleccionada;
- objetivo;
- confianza;
- intención;
- expresión observable;
- afirmaciones a transmitir;
- continuidad esperada;
- explicación causal.

MSPA no puede seleccionar una acción ausente de `available_actions`.

## 9. REPORT

Entrada:

```text
REPORT(runtime, agent, action_result, process_time)
```

Puede actualizar aprendizaje, expectativas, emociones, creencias, relaciones, metas y memoria.

## 10. ADVANCE_TO

No es un tick obligatorio.

Sirve para materializar estado temporal diferido, caducar metas y detectar que el paso del tiempo recomienda una decisión.

`target_time < LastProcessedTime` produce `TimeRegression`.

## 11. EXPORT_AGENT / IMPORT_AGENT

`EXPORT_AGENT` produce un `AgentPsychState` portable y versionado.

El estado exportado incluye, además de su estado psicológico:
- `AgentId` original;
- dominio/escala temporal de origen;
- `exported_at`;
- `RngState`;
- versión de estado.

`IMPORT_AGENT` valida antes de crear estado activo:
1. versión;
2. estructura;
3. namespaces/packs;
4. cuotas;
5. referencias;
6. colisiones de identidad;
7. política de remapeo explícita cuando proceda;
8. compatibilidad/conversión temporal;
9. política de RNG.

Nunca sobrescribe silenciosamente un agente existente.

### Política temporal de importación

Por defecto:

```text
ImportTimePolicy::RebaseRelative
```

Las edades/duraciones relativas a `exported_at` se convierten racionalmente a la escala destino y se vuelven a anclar en `import_time`.

Políticas adicionales:
- `PreserveAbsolute`: sólo si origen y destino declaran dominios compatibles;
- `RejectIncompatible`: rechaza importaciones que exijan conversión.

Nunca se reinterpretan ticks de un dominio como si fueran ticks del otro.

### Política de RNG al importar

Por defecto al trasladar un agente a otro runtime/mundo:

```text
RngImportPolicy::RekeyForDestination
```

Se deriva un stream nuevo desde el `SeedRoot` destino, el `AgentId` efectivo y `stream_kind`.

Para restauración/replay del mismo contexto lógico puede solicitarse explícitamente:

```text
RngImportPolicy::PreserveState
```

La política aplicada forma parte del resultado de importación.

## 12. Registro semántico y batch

El runtime usa `RegistrySnapshot` inmutable durante cada batch.

- el batch captura un snapshot al comenzar;
- nuevas definiciones descubiertas por imports/packs van a staging;
- staging no afecta decisiones del batch actual;
- tras validación puede publicarse como snapshot nuevo entre batches;
- símbolos desconocidos pueden preservarse de forma opaca sin registrarlos durante el batch.

Una llamada individual es semánticamente un **batch de una operación**, por lo que captura los mismos snapshots.

### Orden lógico

Cada entrada batch recibe `input_sequence`, igual a su posición estable en la entrada del host.

El orden canónico es:

```text
(AgentId, input_sequence)
```

Dos operaciones del mismo agente conservan así el orden proporcionado por el host.

### `BatchConflict`

Se produce, como mínimo, cuando:
- un mismo agente tiene entradas con el mismo `input_sequence`;
- el orden proporcionado implica tiempos de procesamiento incompatibles;
- se combinan operaciones de lifecycle y simulación sobre el mismo agente sin un orden resoluble por `input_sequence`.

No se resuelve ningún conflicto usando orden de hash, memoria o scheduling de threads.

## 13. Error contract

### C++

Las operaciones públicas devuelven conceptualmente:

```text
Result<T>
```

Los errores esperables no usan excepciones como control de flujo. Ninguna excepción cruza la API pública.

Códigos mínimos:
- `InvalidArgument`
- `UnknownAgent`
- `DuplicateAgent`
- `InvalidState`
- `InvalidPack`
- `QuotaExceeded`
- `VersionMismatch`
- `NamespaceConflict`
- `TimeRegression`
- `BatchConflict`
- `OutOfMemory`
- `UnsupportedOperation`
- `InternalInvariantViolation`

La primera implementación usa excepciones C++ internamente. Las fronteras públicas capturan `std::bad_alloc` y devuelven `OutOfMemory`.

### C ABI

La fachada devuelve `mspa_status` y utiliza parámetros de salida. Los detalles de error se consultan mediante estructuras o buffers estables definidos por la ABI; nunca se exponen excepciones C++.

## 14. Cuotas

Jerarquía:
1. `HardLimits` de MSPA;
2. `RuntimeLimits` del host;
3. recomendaciones externas.

Si el host omite un límite, se usa `DefaultLimits` versionado.

Un pack no puede aumentar límites del runtime.

## 15. Determinismo

Requisitos:
- RNG independiente por agente;
- orden canónico `(AgentId, input_sequence)`;
- registry/configuración/curvas/reglas inmutables durante batch;
- ausencia de dependencia de scheduling de threads;
- tiempo lógico entero y escala racional;
- política numérica explícita en builds de conformidad;
- `RekeyForDestination` por defecto en traslados entre runtimes.

## 16. Diagnostics mínimos

Desde la primera simulación funcional debe existir una traza compacta capaz de registrar:
- operación;
- agente;
- `process_time` y `event_time` cuando corresponda;
- cambios principales de creencias;
- appraisal resumido;
- emociones relevantes;
- opciones consideradas;
- contribuciones principales de utilidad;
- razón compacta de decisión;
- error/cuota alcanzada.

La instrumentación no modifica el resultado simulacional.

## 17. Reglas de degradación

- sin inferencia: sólo conocimiento directo/testimonial;
- sin memoria: no hay reactivación episódica;
- sin aprendizaje: no se actualizan expectativas causales;
- sin expresión: salida expresiva vacía;
- sin relaciones: appraisal social usa baseline configurable;
- sin ánimo: emociones actúan sin esa modulación.

## 18. Conformidad mínima

La API no se considera estable hasta superar escenarios abstractos que cubran:
- IDs aislados por mundo;
- create/remove;
- inicialización derivada;
- observe/decide/report;
- estímulos tardíos;
- conversión temporal entre dominios;
- urgencia;
- histéresis;
- contradicción de creencias;
- import/export;
- políticas de RNG de importación;
- símbolos desconocidos;
- múltiples runtimes;
- batch vs individual;
- varias operaciones para el mismo agente;
- `BatchConflict`;
- `OutOfMemory` en frontera pública;
- cuotas/entrada hostil;
- determinismo.
