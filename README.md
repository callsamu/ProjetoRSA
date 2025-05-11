# Projeto RSA - Matemática Discreta

Grupo:

- Samuel Brederodes
- Antônio Sérgio
- Almir Dias
- Thiago Araújo


## Rodando a Aplicação

Certifique-se que os build systems `meson`, `cmake` e a biblioteca `gmp` estejam instalados no seu sistema.

Em seguida:

```bash
meson setup builddir
meson compile -C ./builddir
./builddir/rsa
```

A aplicação poderá então ser acessada por `https://localhost:3000`.
