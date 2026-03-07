let project = new Project('Cube-Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addKongDir('shaders');
project.setDebugDir('deployment');

project.addIncludeDir('../../build/Kong-osx-metal-cube');
project.addFile('../../build/Kong-osx-metal-cube/kong.m');

project.flatten();

resolve(project);
