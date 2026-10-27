# Stakeholder Map
``` mermaid
graph TB
    subgraph High Power / Low Interest
        S2[C-Suite / Investors]
    end

    subgraph High Power / High Interest
        S1[Project Sponsor]
        S3[Key Product Manager]
    end

    subgraph Low Power / Low Interest
        S4[General Staff]
    end

    subgraph Low Power / High Interest
        S5[End Users]
        S6[Support Team]
    end

    classDef highP fill:#f9f,stroke:#333,stroke-width:2px;
    class S1,S2,S3 highP;
```
