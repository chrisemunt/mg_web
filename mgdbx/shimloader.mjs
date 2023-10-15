import { createRequire } from "module";
const require = createRequire(import.meta.url);

// Load the correct JavaScript shim
let mod;
if (typeof Bun === 'undefined') {
  mod = await import('mg-dbx-napi/node');
}
else {
  mod = await import('mg-dbx-napi/bun');
}

let mgdbx = mod.mgdbx;
let obj = {};

let arch = process.arch;
if (arch === 'x64' && process.platform === 'win32') arch = 'win';
if (arch === 'x64' && process.platform === 'darwin') arch = 'darwin';

if (['win', 'arm', 'arm64', 'x64', 'darwin'].includes(arch)) {
  let dbx = require('mg-dbx-napi/' + arch);
  obj = mgdbx(dbx);
}

let server = obj.server;
let mglobal = obj.mglobal;
let mclass = obj.mclass;

export {
  server,
  mglobal,
  mclass
};
