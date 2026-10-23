# MSPA Domain Pack Contract v0.1

> Contrato de datos para extender semántica sin introducir lógica específica del consumidor en el core.

## 1. Metadata

Un pack declara:

```text
PackId
NamespaceId
version_major/minor/patch
min_mspa_major
name
dependencies[] (PackId + minimum major/minor)
```

La carga rechaza IDs/namespaces inválidos, versión mínima de MSPA incompatible, duplicados y dependencias ausentes o con versión insuficiente.

## 2. Vocabulario

Puede declarar conceptos y tipos de acción. IDs son opacos al core; el significado específico pertenece al pack/consumidor.

## 3. Predicate Schemas

Cada schema puede declarar roles obligatorios/opcionales, agencia, targets, consequences, appraisal hints, conceptos relacionados, relevancia/consecuencia base e intencionalidad.

El schema describe semántica; no fija una emoción universal.

## 4. Action Schemas y consideraciones

Una acción puede asociarse a una lista de consideraciones por ID. Cada consideración declara peso y curva lineal por tramos. El runtime evalúa sólo consideraciones disponibles/relevantes y mantiene la definición como datos.

## 5. Inferencia

Reglas declarativas expresan premisas `PredicateId + polaridad`, conclusión y certeza base. Son acotadas por budgets de inferencia y no ejecutan código arbitrario.

## 6. Context/Outcome Tags

Los packs pueden declarar vocabulario de tags usado por aprendizaje:

```text
ActionType + ContextTags -> OutcomeTag
```

## 7. Valores iniciales

`initial_values` declara `ConceptId + intensidad [0,1]`. Se almacenan como defaults del registry. El host decide si un `AgentInit` los aplica mediante `use_registry_default_values`.

## 8. Snapshots

Aplicar un pack se realiza sobre `RegistryBuilder`. El snapshot activo no se muta. Sólo tras validación se publica un `RegistrySnapshot` nuevo.

## 9. Seguridad

Un pack no puede:
- ejecutar código nativo arbitrario;
- acceder a filesystem/red desde reglas;
- elevar cuotas del runtime/core;
- introducir clases del host;
- mutar un batch ya iniciado;
- sobrescribir silenciosamente IDs de otro namespace/pack.

## 10. Ubicación

MSPA-Lib contiene el **contrato y parser/estructuras de ejecución del pack**, no packs concretos de consumidores. Los packs concretos viven fuera del core.
