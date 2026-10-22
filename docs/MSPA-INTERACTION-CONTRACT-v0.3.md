# MSPA Interaction Contract v0.3

> Contrato lógico candidato, agnóstico al host.

Esta versión amplía v0.2 con ciclo de vida de agentes, política de errores, múltiples runtimes y reglas explícitas de mutación semántica durante batch.

---

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

---

## 2. Runtime

Cada instancia `Runtime` es independiente y posee su propio estado, agentes, seed, configuración, límites, diagnósticos y snapshot semántico.

No existe estado mutable global obligatorio.

Varias instancias pueden coexistir en un mismo proceso.

---

## 3. Tiempo

Toda operación simulacional relevante usa `PsychTime`:

```text
uint64 logical ticks
```

La escala se fija al crear el runtime y permanece estable durante su vida.

Una llamada con tiempo anterior al último tiempo procesado devuelve `TimeRegression`, salvo mecanismos futuros explícitos de replay/snapshot.

---

## 4. CREATE_AGENT

Entrada conceptual:

```text
CREATE_AGENT(runtime, agent_id, agent_init, time)
```

`AgentInit` puede contener estado completo o parcial, personalidad, valores, relaciones, creencias/memorias iniciales autorizadas, seed y política de defaults.

Puede existir una política genérica de inicialización derivada desde uno o más estados fuente. No supone biología, parentesco ni dominio concreto.

Errores típicos:
- `DuplicateAgent`
- `InvalidArgument`
- `QuotaExceeded`
- `InvalidState`

---

## 5. REMOVE_AGENT

Entrada:

```text
REMOVE_AGENT(runtime, agent_id, policy, time)
```

La eliminación del estado activo no obliga a borrar referencias históricas desde otros agentes. Éstas pueden conservarse como `EntityRef` no resolubles.

Errores típicos:
- `UnknownAgent`
- `InvalidArgument`

---

## 6. HAS_AGENT

Consulta read-only de existencia/registro. No altera simulación ni RNG.

---

## 7. OBSERVE

Entrada:

```text
OBSERVE(runtime, agent, stimulus, time)
```

`Stimulus` puede contener fuente, objetivo, proposiciones, confianza perceptiva, saliencia, namespace, timestamp y metadatos externos validados.

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

---

## 8. DECIDE

Entrada:

```text
DECIDE(runtime, agent, available_actions, current_action?, time)
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

---

## 9. REPORT

Entrada:

```text
REPORT(runtime, agent, action_result, time)
```

Puede actualizar aprendizaje, expectativas, emociones, creencias, relaciones, metas y memoria.

---

## 10. ADVANCE_TO

No es un tick obligatorio.

Sirve para materializar estado temporal diferido, caducar metas y detectar que el paso del tiempo recomienda una decisión.

---

## 11. EXPORT_AGENT / IMPORT_AGENT

`EXPORT_AGENT` produce un `AgentPsychState` portable y versionado.

`IMPORT_AGENT` valida antes de crear estado activo:
1. versión;
2. estructura;
3. namespaces/packs;
4. cuotas;
5. referencias;
6. colisiones de identidad;
7. política de remapeo explícita cuando proceda.

Nunca sobrescribe silenciosamente un agente existente.

---

## 12. Registro semántico y batch

El runtime usa `RegistrySnapshot` inmutable durante cada batch.

- el batch captura un snapshot al comenzar;
- nuevas definiciones descubiertas por imports/packs van a staging;
- staging no afecta decisiones del batch actual;
- tras validación puede publicarse como snapshot nuevo entre batches;
- símbolos desconocidos pueden preservarse de forma opaca sin registrarlos durante el batch.

La equivalencia batch/individual se define contra un orden canónico de agentes/operaciones.

---

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
- `UnsupportedOperation`
- `InternalInvariantViolation`

### C ABI

La fachada devuelve `mspa_status` y utiliza parámetros de salida. Los detalles de error se consultan mediante estructuras/buffers estables definidos por la ABI; nunca se exponen excepciones C++.

---

## 14. Cuotas

Jerarquía:
1. `HardLimits` de MSPA;
2. `RuntimeLimits` del host;
3. recomendaciones del pack.

Si el host omite un límite, se usa `DefaultLimits` versionado.

Un pack no puede aumentar límites del runtime.

---

## 15. Determinismo

Requisitos:
- RNG independiente por agente;
- orden canónico;
- registry/configuración/curvas/reglas inmutables durante batch;
- ausencia de dependencia de scheduling de threads;
- tiempo lógico entero;
- política numérica explícita en builds de conformidad.

---

## 16. Reglas de degradación

- sin inferencia: sólo conocimiento directo/testimonial;
- sin memoria: no hay reactivación episódica;
- sin aprendizaje: no se actualizan expectativas causales;
- sin expresión: salida expresiva vacía;
- sin relaciones: appraisal social usa baseline configurable;
- sin ánimo: emociones actúan sin esa modulación.

---

## 17. Conformidad mínima

La API no se considera estable hasta superar escenarios abstractos que cubran:
- create/remove;
- inicialización derivada;
- observe/decide/report;
- urgencia;
- histéresis;
- contradicción de creencias;
- import/export;
- símbolos desconocidos;
- múltiples runtimes;
- batch vs individual;
- cuotas/entrada hostil;
- determinismo.
